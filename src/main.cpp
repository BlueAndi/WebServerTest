/* MIT License
 *
 * Copyright (c) 2021 Andreas Merkle <web@blue-andi.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @brief  Main entry point
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <esp_log.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Get number of elements in array. */
#define MAIN_ARRAY_NUM(__arr)   (sizeof(__arr)/sizeof((__arr)[0]))

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static void main_handlePageNotFound(AsyncWebServerRequest *request);

/******************************************************************************
 * Variables
 *****************************************************************************/

/** Logging module. */
static const char*              LOG_MODULE              = "main";

/** Serial interface baudrate. */
static const uint32_t           SERIAL_BAUDRATE         = 115200U;

/** Webserver port */
static const uint32_t           WEBSERVER_PORT          = 80U;

/** Web server */
static AsyncWebServer           gWebServer(WEBSERVER_PORT);

/** WiFi station SSID, which to connect to. */
static const char*              WIFI_STATION_SSID       = "...";

/** WiFi station password. */
static const char*              WIFI_STATION_PASSWORD   = "...";

/** Network hostname */
static const char*              NETWORK_HOSTNAME        = "webservertest";

/** Flag, which shows the current wifi connection state. */
static bool                     gIsWiFiConnected        = false; 

/** Count every download of test data. */
static uint32_t                 gNumberOfDownloads      = 0U;

/** Path to all test data. */
static const char*              TEST_DATA_PATH          = "/testData/";

/** Deferred web request. */
static AsyncWebServerRequest*   gDeferredReq            = nullptr;

/******************************************************************************
 * External functions
 *****************************************************************************/

/**
 * Setup the system.
 */
void setup()
{
    /* Setup serial interface. */
    Serial.begin(SERIAL_BAUDRATE);

    /* Enable low level logging. */
    esp_log_level_set("*", ESP_LOG_INFO);

    /* Serve files with static content with enabled cache control.
     * The client may cache files from filesytem for 1 hour.
     */
    (void)gWebServer.serveStatic("/index.html", SPIFFS, "/index.html", "max-age=3600");
    (void)gWebServer.serveStatic("/favicon.png", SPIFFS, "/favicon.png", "max-age=3600");
    (void)gWebServer.serveStatic("/images/", SPIFFS, "/images/", "max-age=3600");
    (void)gWebServer.serveStatic("/js/", SPIFFS, "/js/", "max-age=3600");
    (void)gWebServer.serveStatic("/style/", SPIFFS, "/style/", "max-age=3600");

    /* Register test data in different ways:
     *
     * Use case 1 (uc1): Download in the webserver context.
     * Use case 2 (uc2): Download in main loop context.
     */
    (void)gWebServer.on("/download/uc1/*", HTTP_GET, 
        [](AsyncWebServerRequest* request)
        {
            String filename = request->url().substring(request->url().lastIndexOf("/") + 1U);
            String path     = TEST_DATA_PATH + filename;
            
            ++gNumberOfDownloads;
            ESP_LOG_LEVEL(ESP_LOG_INFO, LOG_MODULE, "%u - %s requested (uc1).", gNumberOfDownloads, path.c_str());

            if (nullptr != request)
            {
                request->send(SPIFFS, path, String(), true);
            }
        });
    (void)gWebServer.on("/download/uc2/*", HTTP_GET, 
        [](AsyncWebServerRequest* request)
        {
            if (nullptr == gDeferredReq)
            {
                gDeferredReq = request;
            }
        });

    /* Redirect any root access to the default page. */
    (void)gWebServer.on("/", [](AsyncWebServerRequest* request) {
        if (nullptr != request)
        {
            request->redirect("/index.html");
        }
    });

    /* Catch all other request. */
    gWebServer.onNotFound(main_handlePageNotFound);

    /* Mount the filesystem. */
    if (false == SPIFFS.begin())
    {
        ESP_LOG_LEVEL(ESP_LOG_ERROR, LOG_MODULE, "Couldn't mount the filesystem.");
    }
    else
    {
        /* Don't store the wifi configuration in the NVS.
         * This seems to cause a reset after a client connected to the access point.
         * https://github.com/espressif/arduino-esp32/issues/2025#issuecomment-503415364
         */
        WiFi.persistent(false);

        /* Start wifi in station mode, incl. LwIP stack. */
        if (false == WiFi.mode(WIFI_MODE_STA))
        {
            ESP_LOG_LEVEL(ESP_LOG_ERROR, LOG_MODULE, "Couldn't initialize wifi station mode.");
        }
        /* Enable automatic wifi reconnect. */
        else if (false == WiFi.setAutoReconnect(true))
        {
            ESP_LOG_LEVEL(ESP_LOG_ERROR, LOG_MODULE, "Enable autom. reconnect failed.");
        }
        else
        {
            /* Start the webserver. */
            gWebServer.begin();

            /* Connect to wifi network. */
            (void)WiFi.begin(WIFI_STATION_SSID, WIFI_STATION_PASSWORD);

            ESP_LOG_LEVEL(ESP_LOG_INFO, LOG_MODULE, "Total filesystem size: %u kb", SPIFFS.totalBytes() / 1024U);
            ESP_LOG_LEVEL(ESP_LOG_INFO, LOG_MODULE, "Used filesystem size: %u kb", SPIFFS.usedBytes() / 1024U);
            ESP_LOG_LEVEL(ESP_LOG_INFO, LOG_MODULE, "System is up.");
        }
    }

    return;
}

/**
 * Main loop, which is called periodically.
 */
void loop()
{
    if (true == WiFi.isConnected())
    {
        if (false == gIsWiFiConnected)
        {
            ESP_LOG_LEVEL(ESP_LOG_INFO, LOG_MODULE, "WiFi connected: %s", WiFi.localIP().toString().c_str());

            gIsWiFiConnected = true;

            if (false == WiFi.setHostname(NETWORK_HOSTNAME))
            {
                ESP_LOG_LEVEL(ESP_LOG_ERROR, LOG_MODULE, "Failed to set hostname.");
            }
        }

        /* Web request pending? */
        if (nullptr != gDeferredReq)
        {
            String filename = gDeferredReq->url().substring(gDeferredReq->url().lastIndexOf("/") + 1U);
            String path     = TEST_DATA_PATH + filename;

            ++gNumberOfDownloads;
            ESP_LOG_LEVEL(ESP_LOG_INFO, LOG_MODULE, "%u - %s requested (uc2).", gNumberOfDownloads, path.c_str());

            gDeferredReq->send(SPIFFS, path, String(), true);
            gDeferredReq = nullptr;
        }
    }
    else
    {
        if (true == gIsWiFiConnected)
        {
            ESP_LOG_LEVEL(ESP_LOG_INFO, LOG_MODULE, "WiFi disconnected.");

            gIsWiFiConnected = false;
        }
    }

    return;
}

/******************************************************************************
 * Local functions
 *****************************************************************************/

/**
 * Handle a request for a non-existing page.
 * 
 * @param[in] request   Web request
 */
static void main_handlePageNotFound(AsyncWebServerRequest *request)
{
    if (nullptr != request)
    {
        request->send(404, "text/plain", "Not found.");
    }
}
