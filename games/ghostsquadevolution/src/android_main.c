/*
 * android_main.c - start Ghost Squad Evolution on Meta Quest.
 *
 * SDL's Java side calls SDL_main. The game tree is not in the APK: it is
 * copied to the headset (quest/install.sh), and the game runs from it just
 * as it does from vsg_l on a desktop. Looked for in turn:
 *
 *   /sdcard/Android/data/<package>/files/vsg_l   (the app's own folder)
 *   /sdcard/GhostSquadEvolution/vsg_l            (needs all-files access,
 *                                                 which install.sh grants)
 *
 * Everything the runtime prints goes to logcat (tag "gse") and to gse.log
 * beside vsg_l. env.txt in vsg_l, one NAME=value per line, sets runtime
 * options (LINDBERGH_MUTE=1 and the like) without rebuilding.
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <android/log.h>
#include <SDL.h>

int game_main(int argc, char **argv);   /* host.c's main, renamed for Android */

static FILE *g_log;

static void *log_pump(void *arg)
{
    FILE *in = fdopen((int)(intptr_t)arg, "r");
    char line[2048];
    while (in && fgets(line, sizeof line, in)) {
        __android_log_write(ANDROID_LOG_INFO, "gse", line);
        if (g_log) { fputs(line, g_log); fflush(g_log); }
    }
    return NULL;
}

static void log_to_logcat(void)
{
    int fd[2];
    if (pipe(fd)) return;
    setvbuf(stderr, NULL, _IOLBF, 0);
    setvbuf(stdout, NULL, _IOLBF, 0);
    dup2(fd[1], 2);
    dup2(fd[1], 1);
    pthread_t t;
    pthread_create(&t, NULL, log_pump, (void *)(intptr_t)fd[0]);
    pthread_detach(t);
}

static void load_env(void)
{
    FILE *f = fopen("env.txt", "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = 0;
        char *eq = strchr(line, '=');
        if (!eq || line[0] == '#') continue;
        *eq = 0;
        setenv(line, eq + 1, 1);
        fprintf(stderr, "[android] %s=%s\n", line, eq + 1);
    }
    fclose(f);
}

static void *run_game(void *arg)
{
    static char *argv[] = { "ghostsquadevolution", "vsg", NULL };
    *(int *)arg = game_main(2, argv);
    return NULL;
}

int SDL_main(int argc, char **argv)
{
    (void)argc; (void)argv;
    const char *own = SDL_AndroidGetExternalStoragePath();
    const char *bases[2] = { own, "/sdcard/GhostSquadEvolution" };
    char dir[1024] = "";
    const char *base = NULL;
    for (int i = 0; i < 2 && !base; i++) {
        if (!bases[i]) continue;
        snprintf(dir, sizeof dir, "%s/vsg_l/vsg", bases[i]);
        if (access(dir, R_OK) == 0) base = bases[i];
    }
    char logpath[1024];
    snprintf(logpath, sizeof logpath, "%s/gse.log", base ? base : own ? own : "/sdcard");
    g_log = fopen(logpath, "w");
    log_to_logcat();
    if (!base) {
        fprintf(stderr, "[android] no game found: copy the game's vsg_l folder to %s/vsg_l with quest/install.sh "
                        "(and allow the app all-files access)\n", bases[1]);
        return 1;
    }
    snprintf(dir, sizeof dir, "%s/vsg_l", base);
    fprintf(stderr, "[android] game directory %s\n", dir);
    if (chdir(dir) != 0) {
        fprintf(stderr, "[android] cannot enter it: %s\n", strerror(errno));
        return 1;
    }
    load_env();

    /* Every guest call is a host call: the game's main thread needs the
     * same deep stack the desktop build links with. */
    pthread_attr_t a;
    pthread_attr_init(&a);
    pthread_attr_setstacksize(&a, 256u << 20);
    pthread_t t;
    int rc = 1;
    if (pthread_create(&t, &a, run_game, &rc) != 0) {
        fprintf(stderr, "[android] cannot start the game thread\n");
        return 1;
    }
    pthread_join(t, NULL);
    return rc;
}
