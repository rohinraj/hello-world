#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <curl/curl.h>

int main(int argc, char *argv[])
{
  CURL *curl;
  CURLcode res;
  FILE *hd_src;
  struct stat file_info;
  curl_off_t fsize;

  if(argc < 3) {
    printf("Usage: %s <file_to_upload> <presigned_url>\n", argv[0]);
    return 1;
  }

  const char *file_path = argv[1];
  const char *url = argv[2];

  /* get the file size of the local file */
  if(stat(file_path, &file_info)) {
    printf("Could not open '%s': %s\n", file_path, strerror(errno));
    return 1;
  }
  fsize = (curl_off_t)file_info.st_size;

  printf("Local file size: %" CURL_FORMAT_CURL_OFF_T " bytes.\n", fsize);

  /* get a FILE * of the same file */
  hd_src = fopen(file_path, "rb");
  if(!hd_src) {
    printf("Could not open '%s' for reading.\n", file_path);
    return 1;
  }

  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* we want to use our own read function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, fread);

    /* enable uploading */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    /* HTTP PUT please */
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);

    /* specify target URL, and note that this URL should include a file
       name, not only a directory */
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* now specify which file to upload */
    curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

    /* provide the size of the upload, we specicially typecast the value
       to curl_off_t since we must be sure to use the correct data size */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)fsize);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    else
      printf("File uploaded successfully!\n");

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  fclose(hd_src);
  curl_global_cleanup();
  return 0;
}
