//
//  main.cpp
//  DecommentTest
//
//  Created by zhy on 19/06/2017.
//  Copyright © 2017 UAMA. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#else // may be UNIX
#include <libgen.h>
#define _MAX_PATH FILENAME_MAX
#endif
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
//using namespace std;


//------------------------------------------------------------------------
// 类型，常量，全局变量的定义
//........................................................................
// typedef and constants
/** 无符号字符型别名 */
typedef unsigned char uchar;

/** 无符号long型别名 */
typedef unsigned long ulong;

const int LINESIZE=8192;

/** C++状态 */
enum cpp_state_e {
    BLANK,				// space 空格
    IDNAME,				// identifier or reserved-word  识别码和保留字
    PAREN,				// parenthesis. "[", "]", "{", "}", "(", ")" 括号，包括中、大、小
    SEPARATOR,			// separator or char/string closer. ",", ";", "'", """ 分隔符和字符及字符串结束符
    OPERATOR,			// operator or others. "=", "==", "+", "+=", "++", ... 操作符或其它
    CPP_COMMENT,		// C++ single comment. "//..." //单行注释
    C_COMMENT,			// C comment block.    "/* ... */" //块注释
    STRING_CONSTANT,	// string constant. ""..."" //字符串常量
    CHAR_CONSTANT,		// char constant.   "'...'" //字符常量
    STRING_ESCAPE,		// after "\" in string constant. //在字符串中的，在\之后的空格
    CHAR_ESCAPE,		// after "\" in char constant. //在字符常量中的，在\之后的空格
};

//........................................................................
// global variables

/** -b: keep blank line */
bool gIsKeepBlankLine = false;

/** -i: keep indent spaces */
bool gIsKeepIndent = false;

/** -q: remove quoted string */
bool gIsRemoveQuotedString = false;

/** -n: print line number */
bool gIsPrintNumber = false;

/** -s: output to stdout */
bool gIsStdout = false;

/** -r: recursive search */
bool gIsRecursive = false;

/** -d<DIR>: output folder */
const char* gOutDir = NULL;

//........................................................................
// messages
/** short help-message */
const char* gUsage  = "usage :decomment [-h?biqnsr] [-d<DIR>] file1.cpp file2.cpp ...\n";

/** detail help-message for options and version */
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

//------------------------------------------------------------------------
/** 确定是否字母 */
inline bool IsAlnum(int c)
{
    return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

/** 它确定是否可分辨名称字符（下划线或字母数字） */
inline bool IsIdNameChar(int c)
{
    return (c == '_' || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

/** 判断是否是空白 */
inline bool IsSpace(int c)
{
    return (c <= ' '); // 这是一个大胆的决定，这足够了，因为目标是一个文本.
}

/** 判断是否是括号字符 */
inline bool IsParen(int c)
{
    return (c == '[' || c == ']' || c == '{' || c == '}' || c == '(' || c == ')');
}

/** 判断是否是一元运算符 */
inline bool IsUnaryOp(int c)
{
    return (c == '&' || c == '*' || c == '+' || c == '-' || c == '~' || c == '!');
}

void DecommentWildMain(const char* fname);
char *getExtension(const char *path);

//读取文件列表，采用递归的方式把目录下的文件全部遍历
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
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;
        else if(ptr->d_type == 8)    ///file
        {
            
            char myfname[_MAX_PATH+100];	// 扩展100个字符
            strcpy(myfname, basePath);
            strcat(myfname, "/");
            strcat(myfname, ptr->d_name);
            
            char *extension = getExtension(myfname);
            
            if (extension != NULL) {
                printf("%s\n",myfname);
                DecommentWildMain(myfname);
            }
        }
        else if(ptr->d_type == 10)    ///link file
            ;
        //            printf("link file:d_name:%s/%s\n",basePath,ptr->d_name);
        else if(ptr->d_type == 4)    ///dir
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

//获取文件的扩展名
char *getExtension(const char *path)
{
    char myPath[_MAX_PATH+100];	// 扩展100个字符
    strcpy(myPath, path);
    char *ext=strrchr(myPath,'.');
    if (ext)
    {
        *ext='\0';
        ext++;
    }
    if (strcmp(ext, "h") == 0 || strcmp(ext, "m") == 0 || strcmp(ext, "cpp") == 0) { //此处只处理扩展名.h和.m的文件
        return ext;
    }
    return NULL;
}

//------------------------------------------------------------------------
/** C++源代码，以除去换行和“额外空间”和“注释”. */
/*
 fname 输入文件
 line 行数
 state 当前状态
 *d 行字符串数组
 *s 当前行字符串
 */
void DecommentLine(const char* fname, int line, cpp_state_e& state, char* d, const char* s, char myline[LINESIZE])
{
    uchar c;
    char* top = d;
    cpp_state_e lastToken = state; //上一次的标识
    bool needSpace = false;
    bool isMacro = false; //是否是宏
    while ((c = (uchar)*s++) != '\0') { //'\0'代表结束符，c是s指针的前一个字符
        
        
        if (c == '\\' && *s == '\n') { // 行合并的规范.
            if (state == CPP_COMMENT) { //单行注释
                // 単行コメントから次行への行併合指定は何かの誤りなので警告を出して無視する。
                // 漢字第二バイトの文字コードが 0x5c の場合も拾ってしまうが、
                // 英語環境でコンパイルすると併合指定扱いになる怪しいケースなので、警告を出しておく。
                // 从评论到下一行，指定行合并时忽略，因为一些错误的警告。
                // 第二个字节的汉字字符代码 0x5c 而且它会选择的情况下、
                // 由于由合并指定的处理可疑的情况下，以全英文的环境进行编译，保持警告。
                fprintf(stderr, "%s(%d) !!!warning: line-marge-mark '\\' at end of single comment. ignore it.\n",
                        fname, line);
                state = BLANK;
            }
            else if (state == C_COMMENT) { //块注释
                // 在块注释、指定到下一行，但该行合并似乎是一个错误、
                // 因为下一行也是相同的评论、没有问题的地方合并。因此，忽视。
            }
            else {
                *d++ = c;	// c='\\'
            }
            
            ++s; continue; // 行末尾记号'\n'前进到下一个(可能字符串结束)
        }
        
        switch (state) {
            case C_COMMENT:
                if (c == '*' && *s == '/') {
                    state = BLANK; ++s;
                }
                continue;
                
            case CPP_COMMENT:
                if (c == '\n') { //单行注释，保留原有换行符
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
                        continue; // skip output
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
                        continue; // skip output
                }
                break;
                state = STRING_CONSTANT;
                if (gIsRemoveQuotedString)
                    continue; // skip output
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
                if (c == '\n' || IsSpace(c)) { //保留原有换行符或空格
                    *d++ = c;
                    continue;
                }
                if (c == '#') {
                    isMacro = true;
                }
                if (isMacro && lastToken == IDNAME && (c == '"' || c == '\'' || c == '(' || IsUnaryOp(c)) && d > top) {
                    // Don't remove a space after ID/if in following cases:
                    //  #define ID "abc"
                    //  #define ID 'c'
                    //  #define ID (-1)
                    //  #define ID -1
                    //  #if -1
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
//                else if (IsIdNameChar(c)) {
//                    if (lastToken == IDNAME && state == BLANK && d > top) {
//                        needSpace = true; // IDNAME之间的空白的段落作为记号有意义的，所以需要.
//                    }
//                    lastToken = state = IDNAME;
//                    break;
//                }
                else {
                    if (lastToken == OPERATOR && state == BLANK && d > top) {
                        // a. "i++ + j" 和 "i + ++j" 除去运算符之间的空格，无论哪一个，都会变成"i+++j"，但意思就变了
                        // b. "a / *p;" 除去运算符之间的空白，于是表达式产生了注释开始的记号
                        // c. "a & &i;" 除去运算符之间的空白，于是"&"有"&&"的化身，表达式的意义就变化了
                        // d. "a ? b : ::c;" 除去运算符间的空格，于是": ::c"变成":::c"，产生了语法错误
                        // e. "vector<vector<int> >" ">"间的空格去除的话会导致语法错误.
                        // 这些符号之间的空白是定界符号的意思.
                        // “有问题的操作运算答的组合”是很难的,
                        // 对于有问题的情况，判定为“必要的空白”.
                        int c0 = (uchar) d[-1];
                        if ((c0 == '/' && c == '*') || (c0 == '*' && c == '/')
                            || c0 == c  /* "+ +", "& &", ": ::", "> >"... */) {
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
                    if (needSpace == false) { //保留原有换行符和空格，以防止各种情况的发生，如运算符，宏定义等
                        *d++ = c;
                    }
                    state = BLANK;
                    continue;
                }
                goto parse_token;
        }//.endswitch state
        if (needSpace) {
            *d++ = ' '; needSpace = false;
        }
        *d++ = c;
    }//.endwhile s
    *d = '\0';
    if (state == CPP_COMMENT)
        state = BLANK;	// 安全策略.
}


//------------------------------------------------------------------------
/** 相对于从fin输入删除注释和多余的空白，并将其输出到fout. */
void DecommentFile(const char* fname, FILE* fin, FILE* fout)
{
    char buf[LINESIZE];
    char line[LINESIZE];
    cpp_state_e cppState = BLANK;
    
    char lastLine[LINESIZE];
    for (int i = 0; fgets(buf, sizeof(buf), fin) != NULL; ++i) { //fgets每次从文件中读取一行，下次读取会沿着上次的行数往下读取
        char* s = buf;
        if (gIsKeepIndent) {
            while (*s == ' ' || *s == '\t')
                ++s;
        }
        
        if (i < 0) { //保留前8行，不做任何修改
            fputs(s, fout); //输入到目标文件中
            continue;
        }
        
        DecommentLine(fname, i+1, cppState, line, s, line);
        
        if (!gIsKeepBlankLine && !line[0])
            continue;
        if (gIsPrintNumber)
            fprintf(fout, "%07d:", i+1);
        if (buf < s)
            fwrite(buf, 1, s - buf, fout); // indent
        
        if (*lastLine == '\n' && *line == '\n') {
            //
        } else {
            fputs(line, fout); // decommented line，向目录文件输入已经处理过的该行
        }
        *lastLine = *line;
        
        
        if (cppState != C_COMMENT) {
//            putc('\n', fout);
        }
    }
}

//------------------------------------------------------------------------
/** 打开输入文件.
 * 打开失败时，到此为止，不需要返回.
 */
FILE* OpenInput(const char* fname)
{
    FILE* fp = fopen(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "can't open input file: %s\n", fname);
        exit(EXIT_FAILURE);
    }
    return fp;
}

//------------------------------------------------------------------------
/** 打开输出文件.
 * 输出文件的名称，假设你添加extname来传递文件名末尾.
 * 打开失败时，到此为止，不需要返回.
 */
FILE* OpenOutput(const char* inputfname, const char* extname)
{
    char fname[_MAX_PATH+100];	// 扩展100个字符
    if (gOutDir) {
#ifdef _WIN32
        char base[_MAX_PATH];
        char ext[_MAX_PATH];
        _splitpath(inputfname, NULL, NULL, base, ext);
        _makepath(fname, NULL, gOutDir, base, ext);
#else // may be UNIX
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

//------------------------------------------------------------------------
/** 显示用法和错误消息后退出 */
void error_abort(const char* msg)
{
    fputs(gUsage, stderr);
    if (msg)
        fputs(msg, stderr);
    exit(EXIT_FAILURE);
}

//------------------------------------------------------------------------
/** 比较s1和s2是否相等 */
inline bool strequ(const char* s1, const char* s2)
{
    return strcmp(s1, s2) == 0;
}

//------------------------------------------------------------------------
/** fname读取、删除评论和多余空格、fname+".decomment"并输出. */
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
    //    remove(fname);
    
    
    
    char myfname[_MAX_PATH+100];	// 扩展100个字符
    strcpy(myfname, fname);
    strcat(myfname, ".decomment");
    
    //    rename(myfname, fname);
}

//------------------------------------------------------------------------
/** 通配符扩展并递归搜索 DecommentMain */
void DecommentWildMain(const char* fname)
{
    printf("%s\n", fname);
    
#ifdef _WIN32
    if (strpbrk(fname, "*?") == NULL) {
        //----- 路径名称的处理不包括通配符
        DecommentMain(fname);
    }
    else {
        //----- 路径名称的处理包括通配符
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
                // fprintf(stderr, "decomment: %s\n", find.name);
                DecommentMain(path);
            } while (_findnext(h, &find) == 0);
            _findclose(h);
        }
        if (!gIsRecursive)
            return;
        
        // 找到子文件夹，并递归每一个
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
                // fprintf(stderr, "decomment recursive: %s\n", path);
                DecommentWildMain(path); // 递归调用.
            } while (_findnext(h, &find) == 0);
            _findclose(h);
        }
    }
#else // may be UNIX: already glob by shell. don't care.
    DecommentMain(fname);
#endif
}

//------------------------------------------------------------------------
/** 功能入口 */
int main(int argc, char* argv[])
{
    //遍历输入参数
    while (argc > 1 && argv[1][0]=='-') {
        char* sw = &argv[1][1];
        if (strcmp(sw, "help") == 0)
            goto show_help;
        else {
            do {
                switch (*sw) {
                    case 'h': case '?':
                    show_help:			error_abort(gUsage2);
                        break;
                    case 'b': //保持空行
                        gIsKeepBlankLine = true;
                        break;
                    case 'i': //保持缩进
                        gIsKeepIndent = true;
                        break;
                    case 'q': //移除带引号的字符串
                        gIsRemoveQuotedString = true;
                        break;
                    case 'n': //是否打印数字
                        gIsPrintNumber = true;
                        break;
                    case 's': //标准输出
                        gIsStdout = true;
                        break;
                    case 'r': //递归（对源代码做了更改，已自动递归）
                        gIsRecursive = true;
                        break;
                    case 'd': //输出目录
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
    
    //--- 读取命令行的输入的目录.
    for (int i = 1; i < argc; i++)
    {
        readFileList(argv[i]);
    }
    
    return EXIT_SUCCESS;
}

// decomment.cpp - end.
