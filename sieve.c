#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>

#define MAX_LINE_LENGTH 1048576

/**
 * このコマンドのオプション
 */
typedef struct {
    char *placeholder;
    char *linenum_placeholder;
    char *condition;
} command_option;

static char default_placeholder[] = "{}";
static char default_linenum_placeholder[] = "{linenum}";

/**
 * <origin> 文字列中の <target> 部分文字列を <replaced> 文字列で置き換える
 *
 * @param[in,out] origin 全体文字列
 * @param[in] replaced_str_length 全体文字列の最大長
 * @param[in] target 置換対象部分文字列
 * @param[in] replaced 置換後部分文字列
 */
void str_replace(char *origin, size_t replaced_str_length, const char *target, const char *replaced);

void parse_option(int argc, char *argv[], command_option *option);


void str_replace(char *origin, size_t replaced_str_length, const char *target, const char *replaced)
{
    size_t target_length   = strlen(target);
    size_t replaced_length = strlen(replaced);

    // 置換対象文字列の長さが 0 のときはエラー
    if (target_length == 0) {
        fprintf(stderr, "置換対象文字列の長さが 0\n");
        exit(1);
    }

    // 文字列 (origin) 中に何度、置換対象文字列 (target) が出現するか数える
    size_t target_count = 0;
    const char *cursor = origin;
    while ((cursor = strstr(cursor, target)) != NULL) {
        target_count++;
        cursor += target_length;
    }

    // <置換後文字列 (replaced) の長さ> - <置換対象文字列 (target) の長さ >
    // が 1 回の置換で大きくなる全体長さ
    size_t growth_length = replaced_length - target_length;
    size_t total_growth_length = growth_length * target_count;

    // 全ての置換後の文字列の長さが与えられた置換後文字列の格納領域以下の長さであることを確認
    if (strlen(origin) + total_growth_length > replaced_str_length) {
        fprintf(stderr, "置換後の文字列の長さが用意された文字配列の長さを越えます\n");
        exit(1);
    }

    // 置換しつつ戻り値を生成
    char original_str[replaced_str_length + 1];
    strcpy((char *) original_str, origin);
    char *prev_cursor     = (char *) original_str;
    char *matched_cursor  = (char *) original_str;
    char *replaced_cursor = origin;
    while ((matched_cursor = strstr(prev_cursor, target)) != NULL) {
        size_t unmatch_length = strlen(prev_cursor) - strlen(matched_cursor);
        memcpy(replaced_cursor, prev_cursor, unmatch_length);
        prev_cursor     += unmatch_length;
        replaced_cursor += unmatch_length;
        strcpy(replaced_cursor, replaced);
        prev_cursor     += target_length;
        replaced_cursor += replaced_length;
    }
    strcpy(replaced_cursor, prev_cursor);
}

void parse_option(int argc, char *argv[], command_option *option)
{
    int c, i;

    // option の初期化
    option->placeholder         = (char *) default_placeholder;
    option->linenum_placeholder = (char *) default_linenum_placeholder;

    // コマンド引数のオプション部分をパース
    while ((c = getopt(argc, argv, "p:l:")) != -1) {
        switch (c) {
        case 'p':  // placeholder: 条件式中の入力行で置き換える文字列の指定
            option->placeholder = optarg;
            break;
        case 'l':  // line number placeholder: 条件式中の行番号で置き換える文字列の指定
            option->linenum_placeholder = optarg;
            break;
        default:
            exit(1);
        }
    }

    // コマンド引数のオプション以外をパース
    // (引数の残りを結合し、条件式を生成する)
    int condition_length = 0;
    for (i=optind; i<argc; i++) condition_length += strlen(argv[i]) + 1;
    char *condition_template = (char *) malloc(sizeof(char));
    char *condition_template_cursor = condition_template;
    for (i=optind; i<argc; i++) {
        strcpy(condition_template_cursor, argv[i]);
        condition_template_cursor += strlen(argv[i]);
        *condition_template_cursor = ' ';
        condition_template_cursor++;
    }
    *condition_template_cursor = '\0';
    option->condition = condition_template;
}

int main(int argc, char *argv[])
{
    char buf[MAX_LINE_LENGTH];
    char condition[MAX_LINE_LENGTH];
    // linenum の最大値から最大文字列長を求め、文字列長とする
    char linenum_string[(int) (log10(UINT_MAX)) + 2];

    // コマンドのオプション、引数を処理する
    command_option option;
    parse_option(argc, argv, &option);

    // 一行ずつ処理する
    unsigned int linenum = 0;
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        // 改行文字を終端文字にして "1 行の文字列" として扱いやすくする
        char *line_end = strchr(buf, '\n');
        if (line_end != NULL) *line_end = '\0';
        // 置換文字列を行で置換して判定式を生成する
        strcpy(condition, option.condition);
        sprintf(linenum_string, "%u", linenum++);
        str_replace(condition, MAX_LINE_LENGTH - 1, option.placeholder, buf);
        str_replace(condition, MAX_LINE_LENGTH - 1, option.linenum_placeholder, linenum_string);

        // 引数をシェルに渡して実行し、終了ステータスが
        // 0 なら 1 行を出力、0 以外なら出力しない
        if (system(condition) == 0) {
          printf("%s\n", buf);
        }
    }
}
