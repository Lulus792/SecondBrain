#include "sb.h"
#include "backup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static int report(SBStatus status) {
    if (status.code == SB_OK) return 0;
    fprintf(stderr, "%s\n", status.message);
    return 1;
}
static int run(int argc, char **argv) {
    SBProjects projects = {0};
    SBProject created;
    SBStatus result;
    if (argc==5 && !strcmp(argv[1],"restore")) {
        result=sb_backup_restore(argv[2],argv[3],argv[4],&created,NULL,NULL);
        if (result.code==SB_OK) printf("%s\n",created.root);
        return report(result);
    }
    if (argc==3 && !strcmp(argv[1],"inspect")) {
        SBBackupInfo info; result=sb_backup_inspect(argv[2],&info,NULL,NULL);
        if (result.code==SB_OK) printf("%s\t%s\t%zu Dateien\t%llu Bytes\n",info.id,info.name,info.files,(unsigned long long)info.bytes);
        return report(result);
    }
    if (argc >= 5 && !strcmp(argv[1], "new")) {
        result = sb_project_create(argv[2], argv[3], argv[4], argc > 5 ? argv[5] : NULL, &created);
        if (result.code == SB_OK) printf("%s\n", created.root);
        return report(result);
    }
    if (argc < 3 || (strcmp(argv[1], "list") && strcmp(argv[1], "context") && strcmp(argv[1], "search") && strcmp(argv[1],"backup")) || (!strcmp(argv[1],"backup") && argc!=5)) {
        fprintf(stderr, "SecondBrain C17\n"
            "  secondbrain-cli new ARBEITSORDNER KENNUNG NAME [PROJEKTORDNER]\n"
            "  secondbrain-cli list ARBEITSORDNER\n"
            "  secondbrain-cli context ARBEITSORDNER KENNUNG\n"
            "  secondbrain-cli search ARBEITSORDNER KENNUNG SUCHTEXT\n"
            "  secondbrain-cli backup ARBEITSORDNER KENNUNG SICHERUNGSDATEI\n"
            "  secondbrain-cli inspect SICHERUNGSDATEI\n"
            "  secondbrain-cli restore SICHERUNGSDATEI ARBEITSORDNER NEUE-KENNUNG\n");
        return 2;
    }
    result = sb_projects_scan(argv[2], &projects);
    if (result.code != SB_OK) return report(result);
    if (!strcmp(argv[1], "list")) {
        bool incomplete=false;
        for (size_t i = 0; i < projects.count; ++i) {
            if (projects.items[i].problem.code!=SB_OK) { incomplete=true; fprintf(stderr,"%s: %s\n",projects.items[i].id,projects.items[i].problem.message); }
            else printf("%s\t%s\n", projects.items[i].id, projects.items[i].name);
        }
        sb_projects_free(&projects);
        return incomplete ? 1 : 0;
    }
    if (argc < 4) { sb_projects_free(&projects); return 2; }
    for (size_t i = 0; i < projects.count; ++i) {
        if (strcmp(projects.items[i].id, argv[3])) continue;
        if (projects.items[i].problem.code!=SB_OK) {
            result=projects.items[i].problem; sb_projects_free(&projects); return report(result);
        }
        if (!strcmp(argv[1],"backup") && argc==5) result=sb_backup_create(&projects.items[i],argv[4],NULL,NULL);
        else if (!strcmp(argv[1], "context")) {
            char *text = NULL;
            result = sb_context_build(&projects.items[i], &text);
            if (result.code == SB_OK) printf("%s", text);
            free(text);
        } else if (argc >= 5) {
            SBNotes notes = {0};
            result = sb_search(&projects.items[i], argv[4], &notes);
            if (result.code == SB_OK)
                for (size_t j = 0; j < notes.count; ++j) printf("%s\t%s\n", notes.items[j].path, notes.items[j].title);
            sb_notes_free(&notes);
        } else result = sb_error(SB_INVALID, "Suchtext fehlt.");
        sb_projects_free(&projects);
        return report(result);
    }
    sb_projects_free(&projects);
    return report(sb_error(SB_NOT_FOUND, "Projekt nicht gefunden."));
}
#ifdef _WIN32
int wmain(int argc, wchar_t **wide_args) {
    char **args = calloc((size_t)argc, sizeof(*args));
    int result = 1;
    if (!args) return 1;
    SetConsoleOutputCP(CP_UTF8);
    for (int i = 0; i < argc; ++i) {
        int size = WideCharToMultiByte(CP_UTF8, 0, wide_args[i], -1, NULL, 0, NULL, NULL);
        args[i] = malloc((size_t)size);
        if (!args[i] || !size) goto cleanup;
        WideCharToMultiByte(CP_UTF8, 0, wide_args[i], -1, args[i], size, NULL, NULL);
    }
    result = run(argc, args);
cleanup:
    for (int i = 0; i < argc; ++i) free(args[i]);
    free(args);
    return result;
}
#else
int main(int argc, char **argv) { return run(argc, argv); }
#endif
