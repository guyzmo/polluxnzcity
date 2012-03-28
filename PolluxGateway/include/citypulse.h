#ifndef __CITYPULSE_H__
#define __CITYPULSE_H__

#include <stdio.h>
#include <string.h>

#include <gcrypt.h>
#include <curl/curl.h>

extern "C" {
char* calculate_sha1(const char* msg, const char* key) {
    // code snippet found on http://ubuntuforums.org/archive/index.php/t-337664.html

    /* Length of message to encrypt */
    int msg_len = strlen( msg )+strlen( key );

    char* message = (char*)malloc(sizeof(char)*msg_len);
    strncpy(message, msg, strlen(msg));
    strncpy(message+strlen(msg), key, strlen(key));

    printf("%d\n", strlen(message));
    printf("%s\n", message);

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
    gcry_md_hash_buffer( GCRY_MD_SHA1, hash, msg, strlen(msg) );

    /* Convert each byte to its 2 digit ascii
     * hex representation and place in out */
    int i;
    for ( i = 0; i < hash_len; i++, p += 2 ) {
        snprintf ( p, 3, "%02x", hash[i] );
    }

//#ifdef VERBOSE
    printf( "hash value: %s\n", out );
//#endif

    free(message);
    return out;
}

int post_to_citypulse(const char* content_string)
{
    int err;

    const char* url_fmt = "http://www2.star-apic.com/citypulse/data/SetArduinoData?s=%s&h=%s";
    //const char* url_fmt = "http://192.168.69.225:42000/SetArduinoData?s=%s&h=%s";
    const char* key = "123456";
    char* hash = calculate_sha1(content_string, key);
    char* url = (char*)malloc(sizeof(char)*strlen(url_fmt)+strlen(key)+strlen(hash));
    char* err_str = (char*)malloc(sizeof(char)*CURLOPT_ERRORBUFFER);

    sprintf(url, url_fmt, key, hash);

    CURL *curl = curl_easy_init(); 
    if(curl) 
    { 
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err_str);
        curl_easy_setopt(curl, CURLOPT_URL, url);
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

        free(hash);
        free(url);
    } else
        return -1;

    return err;
}
}

#endif
