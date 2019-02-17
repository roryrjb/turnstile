// vim: sw=8 ts=8

#include <errno.h>
#include <fcgiapp.h>
#include <fcntl.h>
#include <getopt.h>
#include <mkdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define NAME "turnstile"
#define VERSION "0.0.2"

int
version(char* error)
{
        fprintf(stderr, "%s version %s\n", NAME, VERSION);
        return error == NULL ? 0 : 1;
}

int
usage(char *error)
{
        if (error != NULL) {
                fprintf(stderr, "Error: %s\n\n", error);
        }

        fprintf(stderr, "Usage: %s [OPTIONS]\n", NAME);
        fprintf(stderr, "\nOptions:\n");
        fprintf(stderr, "  --help -h            show help\n");
        fprintf(stderr, "  --version            show version\n");
        fprintf(stderr, "  --host HOSTNAME      set host (default: \"localhost\")\n");
        fprintf(stderr, "  --port PORT          set listening port (default: 9000)\n");
        fprintf(stderr, "  -v[v]                be [more]verbose\n");
        fprintf(stderr, "\n");
        return 0;
}

int
main(int argc, char **argv)
{
        int v = 0;
        char *host = "localhost";
        char *port = "9000";

        static struct option options[] = {
                {"host", required_argument, 0, 0},
                {"port", required_argument, 0, 0},
                {"version", no_argument, 0, 0},
                {"help", no_argument, 0, 0},
                {0, 0, 0, 0}
        };

        int res;
        int index = 0;

        while ((res = getopt_long(argc, argv, "hv", options, &index)) > -1) {
                switch (res) {
                case 0:
                        if (strcmp("host", options[index].name) == 0) {
                                host = optarg;
                        } else if (strcmp("port", options[index].name) == 0) {
                                port = optarg;
                        } else if (strcmp("version", options[index].name) == 0) {
                                return version(NULL);
                        } else if (strcmp("help", options[index].name) == 0) {
                                return usage(NULL);
                        }

                        break;

                case 'h':
                        return usage(NULL);
                        break;

                case 'v':
                        v++;
                        break;
                }
        }

        char address[strlen(host) + strlen(port) + 2];
        strcpy(address, host);
        strcat(address, ":");
        strcat(address, port);

        int sockfd = FCGX_OpenSocket(address, 32);

        if (v >= 1) printf("started: %s\n", address);

        FCGX_Request req;
        FCGX_Init();
        FCGX_InitRequest(&req, sockfd, 0);

        int buffer_size = 4096;
        int header_fd = open("header.html", O_RDONLY);
        int footer_fd = open("footer.html", O_RDONLY);

        char header[buffer_size];
        char footer[buffer_size];

        header[0] = '\0';
        footer[0] = '\0';

        while (read(header_fd, header, buffer_size) > 0);
        while (read(footer_fd, footer, buffer_size) > 0);

        close(header_fd);
        close(footer_fd);

        while (FCGX_Accept_r(&req) == 0) {
                char path[256];
                char *code = "200 OK";
                path[0] = '\0';

                char *document_root = FCGX_GetParam("DOCUMENT_ROOT", req.envp);
                char *document_uri = FCGX_GetParam("DOCUMENT_URI", req.envp);

                strcat(path, document_root);
                strcat(path, document_uri);

                if (v >= 2) printf("trying path '%s'", path);

                char *md_buffer;
                FILE *md_file = fopen(path, "r");

                if (md_file == NULL) {
                        switch (errno) {
                        case ENOENT:
                                code = "404 Not Found";
                                break;
                        default:
                                code = "500 Internal Server Error";
                        }
                }

                if (v >= 2) printf(": %s\n", code);

                FCGX_FPrintF(req.out, "Status: %s\n", code);
                FCGX_FPrintF(req.out, "X-Powered-By: %s/%s\n", NAME, VERSION);
                FCGX_FPrintF(req.out, "Content-Type: text/html\n\n");
                FCGX_FPrintF(req.out, header);

                if (md_file != NULL) {
                        MMIOT *md = mkd_in(md_file, 0);
                        mkd_compile(md, MKD_FENCEDCODE);
                        mkd_document(md, &md_buffer);

                        int i;
                        int len = sizeof(md_buffer) / sizeof(char*);

                        for (i = 0; i < len; i++) {
                                FCGX_FPrintF(req.out, md_buffer);
                        }

                        mkd_cleanup(md);
                        fclose(md_file);
                } else {
                        FCGX_FPrintF(req.out, "<div>");
                        FCGX_FPrintF(req.out, code);
                        FCGX_FPrintF(req.out, "</div>");
                }

                FCGX_FPrintF(req.out, footer);
                FCGX_Finish_r(&req);
        }

        return 0;
}
