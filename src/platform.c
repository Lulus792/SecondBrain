#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include "platform.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#include <wchar.h>

static wchar_t *wide(const char *path) {
    int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, NULL, 0);
    wchar_t *out;
    if (count <= 0) return NULL;
    out = malloc((size_t)count * sizeof(*out));
    if (out) MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, -1, out, count);
    return out;
}

int sb_fs_kind(const char *path) {
    wchar_t *w = wide(path);
    DWORD attributes;
    if (!w) return -1;
    attributes = GetFileAttributesW(w);
    free(w);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        DWORD e = GetLastError();
        return e == ERROR_FILE_NOT_FOUND || e == ERROR_PATH_NOT_FOUND ? 0 : -1;
    }
    if (attributes & FILE_ATTRIBUTE_REPARSE_POINT) return 3;
    return attributes & FILE_ATTRIBUTE_DIRECTORY ? 2 : 1;
}

SBStatus sb_fs_list(const char *path, SBVisit visitor, void *userdata) {
    WIN32_FIND_DATAW item;
    HANDLE handle;
    wchar_t *pattern;
    char joined[SB_PATH_CAP], name[SB_PATH_CAP];
    SBStatus result = sb_path_join(joined, sizeof(joined), path, "*");
    if (result.code != SB_OK) return result;
    pattern = wide(joined);
    if (!pattern) return sb_error(SB_INVALID, "Ungültiger UTF-8-Pfad.");
    handle = FindFirstFileW(pattern, &item);
    free(pattern);
    if (handle == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND && sb_fs_kind(path) == 2) return sb_ok();
        return sb_error(SB_IO, "Ordner kann nicht gelesen werden: %s", path);
    }
    do {
        int kind;
        if (!wcscmp(item.cFileName, L".") || !wcscmp(item.cFileName, L"..")) continue;
        if (!WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, item.cFileName, -1,
                                 name, (int)sizeof(name), NULL, NULL)) {
            result = sb_error(SB_LIMIT, "Dateiname kann nicht dargestellt werden.");
            break;
        }
        kind = item.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ? 3 :
               item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? 2 : 1;
        result = visitor(name, kind, userdata);
        if (result.code != SB_OK) break;
    } while (FindNextFileW(handle, &item));
    if (result.code == SB_OK && GetLastError() != ERROR_NO_MORE_FILES)
        result = sb_error(SB_IO, "Ordnerliste unvollständig: %s", path);
    FindClose(handle);
    return result;
}

SBStatus sb_fs_read(const char *path, char **out, size_t *length) {
    wchar_t *w = wide(path);
    HANDLE file;
    LARGE_INTEGER size;
    DWORD got;
    char *text;
    size_t total = 0;
    *out = NULL;
    if (!w) return sb_error(SB_INVALID, "Ungültiger UTF-8-Pfad.");
    file = CreateFileW(w, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    free(w);
    if (file == INVALID_HANDLE_VALUE)
        return sb_error(sb_fs_kind(path) == 0 ? SB_NOT_FOUND : SB_IO,
                        "Datei kann nicht gelesen werden: %s", path);
    if (!GetFileSizeEx(file, &size) || size.QuadPart < 0 ||
        (unsigned long long)size.QuadPart > SB_TEXT_LIMIT) {
        CloseHandle(file);
        return sb_error(SB_LIMIT, "Datei überschreitet die Textgrenze (16 MiB).");
    }
    text = malloc((size_t)size.QuadPart + 1);
    if (!text) { CloseHandle(file); return sb_error(SB_MEMORY, "Nicht genug Arbeitsspeicher."); }
    while (total < (size_t)size.QuadPart) {
        if (!ReadFile(file, text + total, (DWORD)((size_t)size.QuadPart - total), &got, NULL) || !got) {
            free(text); CloseHandle(file); return sb_error(SB_IO, "Datei wurde nicht vollständig gelesen.");
        }
        total += got;
    }
    CloseHandle(file);
    text[total] = 0;
    *out = text; *length = total;
    return sb_ok();
}

SBStatus sb_fs_write_new(const char *path, const char *data, size_t length) {
    wchar_t *w = wide(path);
    HANDLE file;
    DWORD written;
    size_t total = 0;
    SBStatus result = sb_ok();
    if (!w) return sb_error(SB_INVALID, "Ungültiger UTF-8-Pfad.");
    file = CreateFileW(w, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        free(w);
        return sb_error(sb_fs_kind(path) != 0 ? SB_EXISTS : SB_IO,
                        "Datei konnte nicht angelegt werden: %s", path);
    }
    while (total < length) {
        if (!WriteFile(file, data + total, (DWORD)(length - total), &written, NULL) || !written) {
            result = sb_error(SB_IO, "Datei konnte nicht vollständig geschrieben werden."); break;
        }
        total += written;
    }
    if (result.code == SB_OK && !FlushFileBuffers(file))
        result = sb_error(SB_IO, "Datei konnte nicht dauerhaft gespeichert werden.");
    CloseHandle(file);
    if (result.code != SB_OK) DeleteFileW(w);
    free(w);
    return result;
}

SBStatus sb_fs_replace(const char *from, const char *to) {
    wchar_t *a = wide(from), *b = wide(to);
    bool ok = a && b && MoveFileExW(a, b, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    free(a); free(b);
    return ok ? sb_ok() : sb_error(SB_IO, "Datei konnte nicht ausgetauscht werden: %s", to);
}

SBStatus sb_fs_move_new(const char *from, const char *to) {
    wchar_t *a = wide(from), *b = wide(to);
    bool ok = a && b && MoveFileExW(a, b, MOVEFILE_WRITE_THROUGH);
    free(a); free(b);
    return ok ? sb_ok() : sb_error(sb_fs_kind(to) ? SB_EXISTS : SB_IO, "Datei konnte nicht verschoben werden.");
}

SBStatus sb_fs_mkdir(const char *path) {
    wchar_t *w = wide(path);
    bool ok = w && CreateDirectoryW(w, NULL);
    free(w);
    return ok ? sb_ok() : sb_error(sb_fs_kind(path) ? SB_EXISTS : SB_IO, "Ordner konnte nicht angelegt werden: %s", path);
}

SBStatus sb_fs_remove(const char *path) {
    wchar_t *w = wide(path);
    bool ok = w && DeleteFileW(w);
    free(w);
    return ok ? sb_ok() : sb_error(SB_IO, "Temporäre Datei konnte nicht entfernt werden.");
}

SBStatus sb_fs_absolute(const char *path, char *out, size_t capacity) {
    wchar_t *w = wide(path), full[SB_PATH_CAP];
    DWORD size = w ? GetFullPathNameW(w, SB_PATH_CAP, full, NULL) : 0;
    free(w);
    if (!size || size >= SB_PATH_CAP ||
        !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, full, -1, out,
                             (int)capacity, NULL, NULL))
        return sb_error(SB_LIMIT, "Absoluter Pfad zu lang oder ungültig.");
    for (char *p = out; *p; ++p) if (*p == '\\') *p = '/';
    return sb_ok();
}
unsigned long sb_process_id(void) { return GetCurrentProcessId(); }
SBStatus sb_fs_home(char *out, size_t capacity) {
    wchar_t home[SB_PATH_CAP];
    DWORD length = GetEnvironmentVariableW(L"USERPROFILE", home, SB_PATH_CAP);
    if (!length || length >= SB_PATH_CAP ||
        !WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, home, -1, out, (int)capacity, NULL, NULL))
        return sb_error(SB_IO, "Benutzerordner ist nicht erreichbar.");
    for (char *p = out; *p; ++p) if (*p == '\\') *p = '/';
    return sb_ok();
}

#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int sb_fs_kind(const char *path) {
    struct stat item;
    if (lstat(path, &item) != 0) return errno == ENOENT ? 0 : -1;
    if (S_ISLNK(item.st_mode)) return 3;
    if (S_ISDIR(item.st_mode)) return 2;
    return S_ISREG(item.st_mode) ? 1 : 3;
}

SBStatus sb_fs_list(const char *path, SBVisit visitor, void *userdata) {
    DIR *directory = opendir(path);
    struct dirent *entry;
    SBStatus result = sb_ok();
    char full[SB_PATH_CAP];
    if (!directory) return sb_error(SB_IO, "Ordner kann nicht gelesen werden: %s", path);
    for (;;) {
        errno = 0;
        entry = readdir(directory);
        if (!entry) {
            if (errno) result = sb_error(SB_IO, "Ordnerliste unvollständig: %s", path);
            break;
        }
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        result = sb_path_join(full, sizeof(full), path, entry->d_name);
        if (result.code != SB_OK) break;
        result = visitor(entry->d_name, sb_fs_kind(full), userdata);
        if (result.code != SB_OK) break;
    }
    closedir(directory);
    return result;
}

SBStatus sb_fs_read(const char *path, char **out, size_t *length) {
    FILE *file = fopen(path, "rb");
    long size;
    char *text;
    *out = NULL;
    if (!file) return sb_error(errno == ENOENT ? SB_NOT_FOUND : SB_IO,
                               "Datei kann nicht gelesen werden: %s", path);
    if (fseek(file, 0, SEEK_END) || (size = ftell(file)) < 0) {
        fclose(file); return sb_error(SB_IO, "Dateigröße konnte nicht gelesen werden.");
    }
    if ((unsigned long)size > SB_TEXT_LIMIT) {
        fclose(file); return sb_error(SB_LIMIT, "Datei überschreitet die Textgrenze (16 MiB).");
    }
    rewind(file);
    text = malloc((size_t)size + 1);
    if (!text) { fclose(file); return sb_error(SB_MEMORY, "Nicht genug Arbeitsspeicher."); }
    if (fread(text, 1, (size_t)size, file) != (size_t)size || ferror(file)) {
        free(text); fclose(file); return sb_error(SB_IO, "Datei wurde nicht vollständig gelesen.");
    }
    fclose(file);
    text[size] = 0;
    *out = text; *length = (size_t)size;
    return sb_ok();
}

SBStatus sb_fs_write_new(const char *path, const char *data, size_t length) {
    int file = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    SBStatus result = sb_ok();
    size_t total = 0;
    if (file < 0) return sb_error(errno == EEXIST ? SB_EXISTS : SB_IO,
                                 "Datei konnte nicht angelegt werden: %s", path);
    while (total < length) {
        ssize_t written = write(file, data + total, length - total);
        if (written < 0 && errno == EINTR) continue;
        if (written <= 0) { result = sb_error(SB_IO, "Schreibfehler: %s", path); break; }
        total += (size_t)written;
    }
    if (result.code == SB_OK && fsync(file)) result = sb_error(SB_IO, "Datei konnte nicht synchronisiert werden.");
    if (close(file) && result.code == SB_OK) result = sb_error(SB_IO, "Datei konnte nicht geschlossen werden.");
    if (result.code != SB_OK) unlink(path);
    return result;
}

SBStatus sb_fs_replace(const char *from, const char *to) {
    return rename(from, to) == 0 ? sb_ok() : sb_error(SB_IO, "Datei konnte nicht ausgetauscht werden: %s", to);
}

SBStatus sb_fs_move_new(const char *from, const char *to) {
    /* An exclusive hard link prevents replacing an archive created concurrently. */
    if (link(from, to)) return sb_error(errno == EEXIST ? SB_EXISTS : SB_IO, "Archiv konnte nicht angelegt werden.");
    if (unlink(from)) return sb_error(SB_IO, "Archiv angelegt, Original konnte nicht entfernt werden.");
    return sb_ok();
}

SBStatus sb_fs_mkdir(const char *path) {
    return mkdir(path, 0700) == 0 ? sb_ok() :
        sb_error(errno == EEXIST ? SB_EXISTS : SB_IO, "Ordner konnte nicht angelegt werden: %s", path);
}

SBStatus sb_fs_remove(const char *path) {
    return unlink(path) == 0 ? sb_ok() : sb_error(SB_IO, "Temporäre Datei konnte nicht entfernt werden.");
}

SBStatus sb_fs_absolute(const char *path, char *out, size_t capacity) {
    char cwd[SB_PATH_CAP], joined[SB_PATH_CAP], ancestor[SB_PATH_CAP];
    char *resolved, *slash;
    SBStatus result;
    if (*path == '/') result = sb_path_join(joined, sizeof(joined), path, "");
    else {
        if (!getcwd(cwd, sizeof(cwd))) return sb_error(SB_IO, "Arbeitsordner kann nicht gelesen werden.");
        result = sb_path_join(joined, sizeof(joined), cwd, path);
    }
    if (result.code != SB_OK) return result;
    resolved = realpath(joined, NULL);
    if (resolved) {
        result = sb_path_join(out, capacity, resolved, "");
        free(resolved); return result;
    }
    strcpy(ancestor, joined);
    for (;;) {
        slash = strrchr(ancestor, '/');
        if (!slash) return sb_error(SB_INVALID, "Ungültiger absoluter Pfad.");
        if (slash == ancestor) ancestor[1] = 0;
        else *slash = 0;
        resolved = realpath(ancestor, NULL);
        if (resolved) {
            const char *suffix = joined + strlen(ancestor);
            if (*suffix == '/') ++suffix;
            result = sb_path_join(out, capacity, resolved, suffix);
            free(resolved); return result;
        }
        if (!strcmp(ancestor, "/")) return sb_error(SB_IO, "Pfad kann nicht aufgelöst werden.");
    }
}
unsigned long sb_process_id(void) { return (unsigned long)getpid(); }
SBStatus sb_fs_home(char *out, size_t capacity) {
    const char *home = getenv("HOME");
    if (!home || !*home || !sb_utf8_valid(home, strlen(home)))
        return sb_error(SB_IO, "Benutzerordner ist nicht erreichbar.");
    return sb_path_join(out, capacity, home, "");
}
#endif

SBStatus sb_fs_mkdirs(const char *path) {
    char parent[SB_PATH_CAP];
    char *slash;
    int kind = sb_fs_kind(path);
    SBStatus result;
    if (kind == 2) return sb_ok();
    if (kind != 0) return sb_error(SB_IO, "Pfad ist kein zugänglicher Ordner: %s", path);
    if (strlen(path) >= sizeof(parent)) return sb_error(SB_LIMIT, "Pfad zu lang.");
    strcpy(parent, path);
    slash = strrchr(parent, '/');
    if (slash && slash != parent && !(slash == parent + 2 && parent[1] == ':')) {
        *slash = 0;
        result = sb_fs_mkdirs(parent);
        if (result.code != SB_OK) return result;
    }
    result = sb_fs_mkdir(path);
    return result.code == SB_EXISTS && sb_fs_kind(path) == 2 ? sb_ok() : result;
}
