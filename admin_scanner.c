#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_RESET "\033[0m"
#define MAX_THREADS 10
#define URL_SIZE 256

typedef struct {
    const char *url;
    char *path;
    FILE *output_file;
    pthread_mutex_t *file_lock;
    int found_count;
} ThreadData;

typedef struct {
    char url[URL_SIZE];
    bool found;
} FoundURL;

FoundURL found_urls[1000];
int found_url_count = 0;

int check_admin_panel(const char *url, const char *path) {
    CURL *curl;
    CURLcode res;
    char full_url[256];

    snprintf(full_url, sizeof(full_url), "%s%s", url, path);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, full_url);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // Perform a HEAD request
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; AdminScanner/1.0)"); // Set User-Agent

        res = curl_easy_perform(curl);

        if(res == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if(response_code == 200) {
                for (int i = 0; i < found_url_count; i++) {
                    if (strcmp(found_urls[i].url, full_url) == 0) {
                        return 0;
                    }
                }

                printf(COLOR_GREEN "Accessible admin path found: %s\n" COLOR_RESET, full_url);
                strcpy(found_urls[found_url_count].url, full_url);
                found_urls[found_url_count].found = true;
                found_url_count++;
                return 1;
            } else if(response_code == 404) {
                printf(COLOR_RED "404 Not Found: %s\n" COLOR_RESET, full_url);
            } else {
                printf("Response code %ld for %s\n", response_code, full_url);
            }
        } else {
            printf("Failed to reach %s\n", full_url);
        }

        curl_easy_cleanup(curl);
    }
    return 0;
}

void *thread_check_admin_panel(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    char full_url[256];
    snprintf(full_url, sizeof(full_url), "%s%s", data->url, data->path);

    int found = check_admin_panel(data->url, data->path);
    if (found) {
        pthread_mutex_lock(data->file_lock);
        fprintf(data->output_file, "%s\n", full_url);
        pthread_mutex_unlock(data->file_lock);
        data->found_count++;
    }
    free(data->path);
    free(data);
    return NULL;
}

int main(int argc, char *argv[]) {
    printf("\n========================================\n");
    printf("  Marauder's Map Admin Scanner - par 0liv3vanders\n");
    printf("========================================\n\n");
    printf("\033[0;31mDISCLAIMER: Utilisez cet outil de manière responsable et légale.\n");
    printf("L'auteur décline toute responsabilité en cas d'utilisation abusive.\033[0m\n\n");
    printf("Scanning for admin paths on the target URL...\n\n");

    if(argc != 3) {
        fprintf(stderr, "Usage: %s <url> <path_file>\n", argv[0]);
        return 1;
    }

    const char *url = argv[1];
    const char *path_file = argv[2];
    FILE *file = fopen(path_file, "r");
    if (!file) {
        perror("fopen");
        return 1;
    }

    FILE *output_file = fopen("found_admin_paths.txt", "w");
    if (!output_file) {
        perror("fopen");
        fclose(file);
        return 1;
    }

    pthread_mutex_t file_lock;
    pthread_mutex_init(&file_lock, NULL);

    char path[256];
    pthread_t threads[MAX_THREADS];
    int thread_count = 0;
    int total_requests = 0;
    int found_count = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    while (fgets(path, sizeof(path), file)) {
        path[strcspn(path, "\n")] = 0;

        ThreadData *data = malloc(sizeof(ThreadData));
        data->url = url;
        data->path = malloc(strlen(path) + 1);
        strcpy(data->path, path);
        data->output_file = output_file;
        data->file_lock = &file_lock;
        data->found_count = 0;

        pthread_create(&threads[thread_count++], NULL, thread_check_admin_panel, data);
        total_requests++;

        if (thread_count >= MAX_THREADS) {
            for (int i = 0; i < thread_count; i++) {
                pthread_join(threads[i], NULL);
            }
            thread_count = 0;
        }
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&file_lock);
    fclose(file);
    fclose(output_file);
    curl_global_cleanup();

    printf("\n=== Récapitulatif ===\n");
    printf("Nombre de requêtes: %d\n", total_requests);
    printf("URLs trouvées: %d\n", found_url_count);

    return 0;
}
