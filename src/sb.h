#ifndef SECOND_BRAIN_H
#define SECOND_BRAIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SB_PATH_CAP 4096
#define SB_NAME_CAP 256
#define SB_TEXT_LIMIT (16u * 1024u * 1024u)

typedef enum {
    SB_OK, SB_INVALID, SB_EXISTS, SB_NOT_FOUND, SB_IO, SB_MEMORY, SB_CONFLICT, SB_LIMIT, SB_CANCELLED
} SBCode;

typedef struct { SBCode code; char message[512]; } SBStatus;
typedef struct {
    char id[65];
    char name[SB_NAME_CAP];
    char root[SB_PATH_CAP];
} SBProject;
typedef struct {
    char path[SB_PATH_CAP];
    char title[SB_NAME_CAP];
    char section[32];
} SBNote;
typedef struct { SBProject *items; size_t count; } SBProjects;
typedef struct { SBNote *items; size_t count; } SBNotes;
typedef struct { uint64_t hash; size_t length; bool exists; } SBRevision;

SBStatus sb_ok(void);
SBStatus sb_error(SBCode code, const char *format, ...);
SBStatus sb_path_join(char *out, size_t capacity, const char *directory, const char *name);
bool sb_utf8_valid(const char *text, size_t length);
bool sb_text_valid(const char *text,size_t length);
bool sb_id_valid(const char *id);
SBStatus sb_projects_list(const char *workspace, SBProjects *out);
void sb_projects_free(SBProjects *projects);
SBStatus sb_project_create(const char *workspace, const char *id, const char *name,
                           const char *repository, SBProject *out);
SBStatus sb_notes_list(const SBProject *project, SBNotes *out);
void sb_notes_free(SBNotes *notes);
SBStatus sb_note_load(const SBProject *project, const char *relative,
                     char **text, SBRevision *revision);
SBStatus sb_note_save(const SBProject *project, const char *relative,
                     const char *text, SBRevision expected, SBRevision *saved);
SBStatus sb_note_create(const SBProject *project, const char *section,
                       const char *id, const char *title, SBNote *out);
SBStatus sb_note_archive(const SBProject *project, const char *relative,
                        SBRevision expected, char *archived, size_t capacity);
SBStatus sb_search(const SBProject *project, const char *query, SBNotes *out);
SBStatus sb_context_build(const SBProject *project, char **out);
SBStatus sb_markdown_title(const char *text, char *out, size_t capacity);
uint64_t sb_hash(const char *data, size_t length);
void sb_text_free(char *text);
SBStatus sb_metadata_validate(const char *json,size_t length,char name[SB_NAME_CAP]);
SBStatus sb_metadata_reidentify(const char *json,size_t length,const char *id,char **out,size_t *out_length);

#endif
