#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#else 
#include <libgen.h>
#define _MAX_PATH FILENAME_MAX
#endif
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
typedef unsigned char uchar;
typedef unsigned long ulong;
const int LINESIZE=8192;
enum cpp_state_e {
    BLANK,              
    IDNAME,             
    PAREN,              
    SEPARATOR,          
    OPERATOR,           
    CPP_COMMENT,        
    C_COMMENT,          
    STRING_CONSTANT,    
    CHAR_CONSTANT,      
    STRING_ESCAPE,      
    CHAR_ESCAPE,        
};
bool gIsKeepBlankLine = false;
bool gIsKeepIndent = false;
bool gIsRemoveQuotedString = false;
bool gIsPrintNumber = false;
bool gIsStdout = false;
bool gIsRecursive = false;
const char* gOutDir = NULL;
const char* gUsage  = "usage :decomment [-h?biqnsr] [-d<DIR>] file1.cpp file2.cpp ...\n";
const char* gUsage2 =
"  version: 1.9\n"
"  -h -?      this help\n"
"  -b         keep blank line\n"
"  -i         keep indent spaces\n"
"  -q         remove quoted string\n"
"  -n         print line number of input-file\n"
"  -s         output to stdout instend of files(*.decomment)\n"
"  -r         [WIN32 only] recursive search under the input-file's folder(wildcard needed)\n"
"  -d<DIR>    output to DIR\n"
"  fileN.cpp  input-files. wildcard OK\n"
"\n  supports and source codes are: http://code.google.com/p/cpp-decomment/\n";
;
inline bool IsAlnum(int c)
{
    return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}
inline bool IsIdNameChar(int c)
{
    return (c == '_' || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}
inline bool IsSpace(int c)
{
    return (c <= ' '); 
}
inline bool IsParen(int c)
{
    return (c == '[' || c == ']' || c == '{' || c == '}' || c == '(' || c == ')');
}
inline bool IsUnaryOp(int c)
{
    return (c == '&' || c == '*' || c == '+' || c == '-' || c == '~' || c == '!');
}
void DecommentWildMain(const char* fname);
char *getExtension(const char *path);
int readFileList(const char *basePath)
{
    DIR *dir;
    struct dirent *ptr;
    char base[1000];
    
    if ((dir=opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }
    
    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    
            continue;
        else if(ptr->d_type == 8)    
        {
            
            char myfname[_MAX_PATH+100];    
            strcpy(myfname, basePath);
            strcat(myfname, "/");
            strcat(myfname, ptr->d_name);
            
            char *extension = getExtension(myfname);
            
            if (extension != NULL) {
                printf("%s\n",myfname);
                DecommentWildMain(myfname);
            }
        }
        else if(ptr->d_type == 10)    
            ;
        
        else if(ptr->d_type == 4)    
        {
            memset(base,'\0',sizeof(base));
            strcpy(base,basePath);
            strcat(base,"/");
            strcat(base,ptr->d_name);
            readFileList(base);
        }
    }
    closedir(dir);
    return 1;
}
char *getExtension(const char *path)
{
    char myPath[_MAX_PATH+100]; 
    strcpy(myPath, path);
    char *ext=strrchr(myPath,'.');
    if (ext)
    {
        *ext='\0';
        ext++;
    }
    if (strcmp(ext, "h") == 0 || strcmp(ext, "m") == 0 || strcmp(ext, "cpp") == 0) { 
        return ext;
    }
    return NULL;
}
void DecommentLine(const char* fname, int line, cpp_state_e& state, char* d, const char* s, char myline[LINESIZE])
{
    uchar c;
    char* top = d;
    cpp_state_e lastToken = state; 
    bool needSpace = false;
    bool isMacro = false; 
    while ((c = (uchar)*s++) != '\0') { 
        
        
        if (c == '\\' && *s == '\n') { 
            if (state == CPP_COMMENT) { 
                
                
                
                
                
                
                fprintf(stderr, "%s(%d) !!!warning: line-marge-mark '\\' at end of single comment. ignore it.\n",
                        fname, line);
                state = BLANK;
            }
            else if (state == C_COMMENT) { 
                
                
            }
            else {
                *d++ = c;   
            }
            
            ++s; continue; 
        }
        
        switch (state) {
            case C_COMMENT:
                if (c == '*' && *s == '/') {
                    state = BLANK; ++s;
                }
                continue;
                
            case CPP_COMMENT:
                if (c == '\n') { 
                    *d++ = c;
                    state = BLANK;
                }
                continue;
                
            case STRING_CONSTANT:
                if (c == '"')
                    state = SEPARATOR;
                else {
                    if (c == '/')
                        state = STRING_ESCAPE;
                    if (gIsRemoveQuotedString)
                        continue; 
                }
                break;
                
            case CHAR_CONSTANT:
                if (c == '\'')
                    state = SEPARATOR;
                else if (c == '/')
                    state = CHAR_ESCAPE;
                break;
                
            case STRING_ESCAPE:
                if (c == '"')
                    state = SEPARATOR;
                else {
                    if (c == '/')
                        state = STRING_ESCAPE;
                    if (gIsRemoveQuotedString)
                        continue; 
                }
                break;
                state = STRING_CONSTANT;
                if (gIsRemoveQuotedString)
                    continue; 
                break;
                
            case CHAR_ESCAPE:
                if (c == '\'')
                    state = SEPARATOR;
                else if (c == '/')
                    state = CHAR_ESCAPE;
                break;
                state = CHAR_CONSTANT;
                break;
                
            case BLANK:
                if (c == '\n' || IsSpace(c)) { 
                    *d++ = c;
                    continue;
                }
                if (c == '#') {
                    isMacro = true;
                }
                if (isMacro && lastToken == IDNAME && (c == '"' || c == '\'' || c == '(' || IsUnaryOp(c)) && d > top) {
                    
                    
                    
                    
                    
                    
                    needSpace = true;
                }
            parse_token:
                if (c == '/' && *s == '*') {
                    state = C_COMMENT; ++s;
                    continue;
                }
                else if (c == '/' && *s == '/') {
                    state = CPP_COMMENT; ++s;
                    continue;
                }
                else if (c == '"') {
                    if (lastToken == CHAR_CONSTANT) {
                        lastToken = state = BLANK;
                    } else {
                        lastToken = state = STRING_CONSTANT;
                    }
                    break;
                }
                else if (c == '\'') {
                    if (lastToken == CHAR_CONSTANT) {
                        lastToken = state = BLANK;
                    } else {
                        lastToken = state = CHAR_CONSTANT;
                    }
                    break;
                }
                else if (IsParen(c)) {
                    lastToken = state = PAREN;
                    break;
                }
                else if (c == ',' || c == ';') {
                    lastToken = state = SEPARATOR;
                    break;
                }
                else {
                    if (lastToken == OPERATOR && state == BLANK && d > top) {
                        
                        
                        
                        
                        
                        
                        
                        
                        int c0 = (uchar) d[-1];
                        if ((c0 == '/' && c == '*') || (c0 == '*' && c == '/')
                            || c0 == c  ) {
                            needSpace = true;
                        }
                    }
                    lastToken = state = OPERATOR;
                    break;
                }
                
            case PAREN:
            case SEPARATOR:
            case IDNAME:
            case OPERATOR:
                if (c == '\n' || IsSpace(c)) {
                    if (needSpace == false) { 
                        *d++ = c;
                    }
                    state = BLANK;
                    continue;
                }
                goto parse_token;
        }
        if (needSpace) {
            *d++ = ' '; needSpace = false;
        }
        *d++ = c;
    }
    *d = '\0';
    if (state == CPP_COMMENT)
        state = BLANK;  
}
void DecommentFile(const char* fname, FILE* fin, FILE* fout)
{
    char buf[LINESIZE];
    char line[LINESIZE];
    cpp_state_e cppState = BLANK;
    
    for (int i = 0; fgets(buf, sizeof(buf), fin) != NULL; ++i) { 
        char* s = buf;
        if (gIsKeepIndent) {
            while (*s == ' ' || *s == '\t')
                ++s;
        }
        
        if (i < 0) { 
            fputs(s, fout); 
            continue;
        }
        
        DecommentLine(fname, i+1, cppState, line, s, line);
        
        if (!gIsKeepBlankLine && !line[0])
            continue;
        if (gIsPrintNumber)
            fprintf(fout, "%07d:", i+1);
        if (buf < s)
            fwrite(buf, 1, s - buf, fout); 
        fputs(line, fout); 
        
        if (cppState != C_COMMENT) {
        }
    }
}
FILE* OpenInput(const char* fname)
{
    FILE* fp = fopen(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "can't open input file: %s\n", fname);
        exit(EXIT_FAILURE);
    }
    return fp;
}
FILE* OpenOutput(const char* inputfname, const char* extname)
{
    char fname[_MAX_PATH+100];  
    if (gOutDir) {
#ifdef _WIN32
        char base[_MAX_PATH];
        char ext[_MAX_PATH];
        _splitpath(inputfname, NULL, NULL, base, ext);
        _makepath(fname, NULL, gOutDir, base, ext);
#else 
        char input[_MAX_PATH];
        strcpy(input, inputfname);
        snprintf(fname, _MAX_PATH, "%s/%s", gOutDir, basename(input));
#endif
    }
    else {
        strcpy(fname, inputfname);
    }
    strcat(fname, extname);
    
    FILE* fp = fopen(fname, "w");
    if (fp == NULL) {
        fprintf(stderr, "can't open output file: %s\n", fname);
        exit(EXIT_FAILURE);
    }
    return fp;
}
void error_abort(const char* msg)
{
    fputs(gUsage, stderr);
    if (msg)
        fputs(msg, stderr);
    exit(EXIT_FAILURE);
}
inline bool strequ(const char* s1, const char* s2)
{
    return strcmp(s1, s2) == 0;
}
void DecommentMain(const char* fname)
{
    FILE* fin = OpenInput(fname);
    FILE* fout;
    if (gIsStdout) {
        fout = stdout;
        printf("<<< %s >>> begin.\n", fname);
    }
    else {
        fout = OpenOutput(fname, ".decomment");
    }
    
    DecommentFile(fname, fin, fout);
    
    fclose(fin);
    if (gIsStdout) {
        printf("<<< %s >>> end.\n\n", fname);
    }
    else {
        fclose(fout);
    }
    
    
    
    
    char myfname[_MAX_PATH+100];    
    strcpy(myfname, fname);
    strcat(myfname, ".decomment");
    
    
}
void DecommentWildMain(const char* fname)
{
    printf("%s\n", fname);
    
#ifdef _WIN32
    if (strpbrk(fname, "*?") == NULL) {
        
        DecommentMain(fname);
    }
    else {
        
        char path[_MAX_PATH + 1000];
        char drv[_MAX_DRIVE];
        char dir[_MAX_PATH + 1000];
        char base[_MAX_PATH];
        char ext[_MAX_PATH];
        _splitpath(fname, drv, dir, base, ext);
        
        _finddata_t find;
        long h = _findfirst(fname, &find);
        if (h != -1) {
            do {
                if (find.attrib & _A_SUBDIR)
                    continue;
                _makepath(path, drv, dir, find.name, NULL);
                
                DecommentMain(path);
            } while (_findnext(h, &find) == 0);
            _findclose(h);
        }
        if (!gIsRecursive)
            return;
        
        
        _makepath(path, drv, dir, "*.*", NULL);
        h = _findfirst(path, &find);
        if (h != -1) {
            do {
                if (!(find.attrib & _A_SUBDIR))
                    continue;
                if (strequ(find.name, ".") || strequ(find.name, ".."))
                    continue;
                _makepath(path, drv, dir, find.name, NULL);
                strcat(path, "\\");
                strcat(path, base);
                strcat(path, ext);
                
                DecommentWildMain(path); 
            } while (_findnext(h, &find) == 0);
            _findclose(h);
        }
    }
#else 
    DecommentMain(fname);
#endif
}
int main(int argc, char* argv[])
{
    
    while (argc > 1 && argv[1][0]=='-') {
        char* sw = &argv[1][1];
        if (strcmp(sw, "help") == 0)
            goto show_help;
        else {
            do {
                switch (*sw) {
                    case 'h': case '?':
                    show_help:          error_abort(gUsage2);
                        break;
                    case 'b': 
                        gIsKeepBlankLine = true;
                        break;
                    case 'i': 
                        gIsKeepIndent = true;
                        break;
                    case 'q': 
                        gIsRemoveQuotedString = true;
                        break;
                    case 'n': 
                        gIsPrintNumber = true;
                        break;
                    case 's': 
                        gIsStdout = true;
                        break;
                    case 'r': 
                        gIsRecursive = true;
                        break;
                    case 'd': 
                        gOutDir = sw+1;
                        goto next_arg;
                    default:
                        error_abort("unknown option.\n");
                        break;
                }
            } while (*++sw);
        }
    next_arg:
        ++argv;
        --argc;
    }
    if (argc == 1) {
        error_abort("please specify input file.\n");
    }
    
    
    for (int i = 1; i < argc; i++)
    {
        readFileList(argv[i]);
    }
    
    return EXIT_SUCCESS;
}
