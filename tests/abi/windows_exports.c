#include <remix/remix_c.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void add_path_dll_directories(void) {
  DWORD size = GetEnvironmentVariableW(L"PATH", NULL, 0);
  if (!size) return;
  wchar_t *path = malloc((size + 1) * sizeof(*path));
  if (!path) return;
  DWORD length = GetEnvironmentVariableW(L"PATH", path, size + 1);
  if (!length || length > size) {
    free(path);
    return;
  }
  wchar_t *start = path;
  for (wchar_t *cursor = path;; ++cursor) {
    if (*cursor != L';' && *cursor != L'\0') continue;
    const wchar_t separator = *cursor;
    *cursor = L'\0';
    if (*start) AddDllDirectory(start);
    if (!separator) break;
    start = cursor + 1;
  }
  free(path);
}

int main(int argc, char **argv) {
  if (argc != 3) return 1;
  add_path_dll_directories();
  HMODULE library = LoadLibraryExA(argv[1], NULL,
                                   LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
                                       LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
                                       LOAD_LIBRARY_SEARCH_USER_DIRS);
  if (!library) {
    fprintf(stderr, "LoadLibrary failed: %lu\n", GetLastError());
    return 1;
  }
  FILE *exports = fopen(argv[2], "r");
  if (!exports) return 1;
  char name[256];
  while (fgets(name, sizeof(name), exports)) {
    name[strcspn(name, "\r\n")] = 0;
    if (*name && !GetProcAddress(library, name)) {
      fprintf(stderr, "Missing export: %s\n", name);
      return 1;
    }
  }
  fclose(exports);
  typedef int (REMIXAPI_CALL *Begin)(const char *, int *, int);
  typedef void (REMIXAPI_CALL *WindowSize)(float *, float *);
  Begin begin = (Begin)GetProcAddress(library, "remixapi_imgui_Begin");
  WindowSize windowSize = (WindowSize)GetProcAddress(library, "remixapi_imgui_GetWindowSize");
  float value = 12;
  if (!begin || !windowSize || begin(NULL, NULL, 0)) return 1;
  windowSize(&value, NULL);
  if (value != 0) return 1;
  FreeLibrary(library);
  puts("Windows exports, library loading and inert compatibility widgets passed");
  return 0;
}
