// http://stackoverflow.com/questions/1602398/linux-dlopen-can-a-library-be-notified-when-it-is-loaded

#ifndef __CITYPULSE_H__
#define __CITYPULSE_H__

#include <pollux/types.h>

#include <sstream>

extern "C" {

#include <stdio.h>
#include <string.h>

#include <gcrypt.h>
#include <curl/curl.h>

char* calculate_sha1(const char* msg, const char* key) {
    // code snippet found on http://ubuntuforums.org/archive/index.php/t-337664.html

    char* message = (char*)malloc(sizeof(char)*(strlen(msg)+strlen(key)));
    snprintf(message, strlen(msg)+strlen(key), "%s%s", msg, key);

#ifdef VERBOSE
    printf("calculate_sha1(): msg %d %s\n", strlen(msg), msg);
    printf("calculate_sha1(): key %d %s\n", strlen(key), key);
    printf("calculate_sha1(): msg+key %d %s\n", strlen(message), message);
#endif

    /* Length of resulting sha1 hash - gcry_md_get_algo_dlen
     * returns digest lenght for an algo */
    int hash_len = gcry_md_get_algo_dlen( GCRY_MD_SHA1 );

    /* output sha1 hash - this will be binary data */
    unsigned char hash[ hash_len ];

    /* output sha1 hash - converted to hex representation
     * 2 hex digits for every byte + 1 for trailing \0 */
    char *out = (char *) malloc( sizeof(char) * ((hash_len*2)+1) );
    char *p = out;

    /* calculate the SHA1 digest. This is a bit of a shortcut function
     * most gcrypt operations require the creation of a handle, etc. */
    gcry_md_hash_buffer( GCRY_MD_SHA1, hash, message, strlen(message) );

    /* Convert each byte to its 2 digit ascii
     * hex representation and place in out */
    int i;
    for ( i = 0; i < hash_len; i++, p += 2 ) {
        snprintf ( p, 3, "%02x", hash[i] );
    }

#ifdef VERBOSE
    printf( "hash value: %s\n", out );
#endif

    free(message);
    return out;
}

int post_to_citypulse(const char* content_string, const char* url_fmt, const char* serial, const char* api_key, const char* proxy)
{
    int err;

    char* hash = calculate_sha1(content_string, api_key);
    char* url = (char*)malloc(sizeof(char)*strlen(url_fmt)+strlen(api_key)+strlen(hash));
    char* err_str = (char*)malloc(sizeof(char)*CURLOPT_ERRORBUFFER);

    CURL *curl = curl_easy_init(); 
    if(curl) 
    { 
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err_str);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_PROXY, proxy);
        curl_easy_setopt(curl, CURLOPT_POST, 1); 
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content_string); 
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(content_string)); 
        struct curl_slist *slist = curl_slist_append(NULL, "Content-Type: text/form-data; charset=utf-8");
        err = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist); 
        curl_easy_perform(curl); 
        curl_slist_free_all(slist);
        curl_easy_cleanup(curl); 

        if (err < 0)
            printf("Error pushing to the web: %s\n", err_str);
        
        free(err_str);
        free(hash);
        free(url);
    } else
        return -1;

    return err;
}
}

int citypulse_post(std::vector<string_string_map*>& values_list, string_string_map& config) {
    std::ostringstream val_string;
    std::ostringstream url_fmt;
    const char* proxy;

    if (config.find("post_url") == config.end() or config.find("api_key") == config.end())
        return -2;

    if (config.find("proxy") == config.end())
        proxy = "";
    else
        proxy = config["proxy"].c_str();

    url_fmt<<config["post_url"].c_str()<<"?s=%s&h=%s";

    val_string<<"[";

    for (std::vector<string_string_map*>::iterator val_it = values_list.begin(); val_it != values_list.end();++val_it) {
        val_string<<"{\"k\":\""<<(**val_it)["k"]<<"\"";
        val_string<<",\"v\":"<<(**val_it)["v"];
        val_string<<",\"u\":\""<<(**val_it)["u"]<<"\"";
        val_string<<",\"p\":"<<(**val_it)["p"]<<"}";
        if (val_it+1 != values_list.end())
            val_string<<",";
    }

    val_string<<"]";

    return post_to_citypulse(val_string.str().c_str(), 
                                url_fmt.str().c_str(), 
                                /*serial*/config["api_key"].c_str(),
                                config["api_key"].c_str(),
                                proxy);
}

#endif
