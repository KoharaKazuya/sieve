#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1048576

const char replace_literal[] = "{}";


void str_replace(const char *origin, const char *target, const char *replaced, char *replaced_str, int replaced_str_length)
{
    int target_length = strlen(target);
    int replaced_length = strlen(replaced);

    // 置換対象文字列の長さが 0 のときはエラー
    if (target_length == 0) {
        fprintf(stderr, "置換対象文字列の長さが 0\n");
        exit(1);
    }

    // 文字列 (origin) 中に何度、置換対象文字列 (target) が出現するか数える
    int target_count = 0;
    const char *cursor = origin;
    while ((cursor = strstr(cursor, target)) != NULL) {
        target_count++;
        cursor += target_length;
    }

    // <置換後文字列 (replaced) の長さ> - <置換対象文字列 (target) の長さ >
    // が 1 回の置換で大きくなる全体長さ
    int growth_length = replaced_length - target_length;
    int total_growth_length = growth_length * target_count;

    // 全ての置換後の文字列の長さが与えられた置換後文字列の格納領域以下の長さであることを確認
    if (strlen(origin) + total_growth_length > replaced_str_length) {
        fprintf(stderr, "置換後の文字列の長さが用意された文字配列の長さを越えます\n");
        exit(1);
    }

    // 置換しつつ戻り値を生成
    const char *origin_cursor = origin;
    const char *next_cursor = origin;
    char *replaced_cursor = replaced_str;
    while ((next_cursor = strstr(origin_cursor, target)) != NULL) {
        int unmatch_length = strlen(origin_cursor) - strlen(next_cursor);
        memcpy(replaced_cursor, origin_cursor, unmatch_length);
        origin_cursor   += unmatch_length;
        replaced_cursor += unmatch_length;
        strcpy(replaced_cursor, replaced);
        origin_cursor   += target_length;
        replaced_cursor += replaced_length;
    }
    strcpy(replaced_cursor, origin_cursor);
}

int main(int argc, char **argv)
{
    int i;
    char buf[MAX_LINE_LENGTH];
    char condition[MAX_LINE_LENGTH];

    // 引数が 1 つ以上なければ終了する
    if (argc <= 1) {
        fprintf(stderr, "条件式を指定して下さい\n");
        return 1;
    }

    // 引数を結合し、条件式を生成する
    int condition_length = 0;
    for (i=1; i<argc; i++) condition_length += strlen(argv[i]) + 1;
    char condition_template[condition_length];
    char *condition_template_cursor = condition_template;
    for (i=1; i<argc; i++) {
        strcpy(condition_template_cursor, argv[i]);
        condition_template_cursor += strlen(argv[i]);
        *condition_template_cursor = ' ';
        condition_template_cursor++;
    }
    *condition_template_cursor = '\0';

    // 一行ずつ処理する
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        // 改行文字を終端文字にして "1 行の文字列" として扱いやすくする
        char *line_end = strchr(buf, '\n');
        if (line_end != NULL) *line_end = '\0';
        // 第一引数の置換文字列を行で置換して判定式を生成する
        str_replace(condition_template, replace_literal, buf, condition, MAX_LINE_LENGTH - 1);

        // 引数をシェルに渡して実行し、終了ステータスが
        // 0 なら 1 行を出力、0 以外なら出力しない
        if (system(condition) == 0) {
          printf("%s\n", buf);
        }
    }
}
