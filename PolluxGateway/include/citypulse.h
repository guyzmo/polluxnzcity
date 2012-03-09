#ifndef __CITYPULSE_H__
#define __CITYPULSE_H__

#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

int post_to_citypulse(char* content_string)
{
    CURL *curl = curl_easy_init(); 
    if(curl) 
    { 
        curl_easy_setopt(curl, CURLOPT_URL, "http://www2.star-apic.com//citypulse/data/SetArduinoData?s=123456&h=hashouille"); 
        curl_easy_setopt(curl, CURLOPT_POST, 1); 
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content_string); 
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(content_string)); 
        struct curl_slist *slist = curl_slist_append(NULL, "Content-Type: text/json; charset=utf-8");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist); 
        curl_easy_perform(curl); 
        curl_slist_free_all(slist);
        curl_easy_cleanup(curl); 
    } 
    return 0;
}

#endif
