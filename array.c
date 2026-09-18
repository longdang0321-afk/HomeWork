#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEVELS 20

typedef struct {
    char *data;
    int capacity;
} ArrayTree;

typedef struct {
    int nodes, leaves, height, degree, left, right;
} Info;

static const char *pos;
static int used[26];

static void error(const char *message)
{
    fprintf(stderr, "오류: %s\n", message);
    exit(1);
}

static void spaces(void)
{
    while (isspace((unsigned char)*pos)) pos++;
}

/* 괄호를 읽으며 인덱스 위치에 직접 저장한다. 연결 노드는 만들지 않는다. */
static void parseNode(ArrayTree *a, int index, int level)
{
    spaces();
    if (*pos == ',' || *pos == ')') return;
    if (level > MAX_LEVELS) error("최대 20층까지 지원합니다.");
    if (*pos < 'A' || *pos > 'Z') error("노드 이름은 A~Z입니다.");
    if (used[*pos - 'A']) error("이름이 중복됩니다.");
    used[*pos - 'A'] = 1;
    while (index >= a->capacity) {
        int newCapacity = a->capacity * 2;
        char *temp = realloc(a->data, (size_t)newCapacity);
        if (!temp) error("메모리 할당 실패");
        memset(temp + a->capacity, 0, (size_t)(newCapacity - a->capacity));
        a->data = temp;
        a->capacity = newCapacity;
    }
    a->data[index] = *pos++;
    spaces();
    if (*pos == '(') {
        pos++;
        parseNode(a, 2 * index, level + 1);
        spaces();
        if (*pos == ',') {
            pos++;
            parseNode(a, 2 * index + 1, level + 1);
        }
        spaces();
        if (*pos != ')') error("괄호 또는 자식 수를 확인하세요.");
        pos++;
    }
}

static ArrayTree parse(const char *text)
{
    ArrayTree a = {calloc(2, sizeof(char)), 2};
    if (!a.data) error("메모리 할당 실패");
    pos = text;
    memset(used, 0, sizeof(used));
    spaces();
    if (*pos) parseNode(&a, 1, 1);
    spaces();
    if (*pos) error("입력 뒤에 불필요한 문자가 있습니다.");
    return a;
}

static int exists(const ArrayTree *a, int i)
{
    return i > 0 && i < a->capacity && a->data[i] != '\0';
}

static Info arrayInfo(const ArrayTree *a)
{
    Info s = {0, 0, -1, 0, 0, 0};
    for (int i = 1; i < a->capacity; i++) {
        if (!exists(a, i)) continue;
        int l = exists(a, 2 * i), r = exists(a, 2 * i + 1);
        int depth = 0;
        for (int j = i; j > 1; j /= 2) depth++;
        s.nodes++;
        s.leaves += (l + r == 0);
        s.left += l;
        s.right += r;
        if (depth > s.height) s.height = depth;
        if (l + r > s.degree) s.degree = l + r;
    }
    return s;
}

static void arrayPrint(const ArrayTree *a, int i, int depth)
{
    if (!exists(a, i)) return;
    arrayPrint(a, 2 * i + 1, depth + 1);
    printf("%*s%c\n", depth * 4, "", a->data[i]);
    arrayPrint(a, 2 * i, depth + 1);
}

static int arrayComplete(const ArrayTree *a, int n)
{
    for (int i = 1; i <= n; i++)
        if (!exists(a, i)) return 0;
    return 1;
}

static void printInfo(Info s, int complete)
{
    int perfect = s.nodes == (1 << (s.height + 1)) - 1;
    int skew = s.nodes == s.height + 1 && (s.left == 0 || s.right == 0);
    printf("전체 노드: %d / 단말: %d / 비단말: %d\n",
           s.nodes, s.leaves, s.nodes - s.leaves);
    printf("높이: %d / 차수: %d\n", s.height, s.degree);
    printf("완전: %s / 포화: %s / 편향: %s\n",
           complete ? "예" : "아니오", perfect ? "예" : "아니오",
           skew ? "예" : "아니오");
}

static void relation(const char *label, char name)
{
    if (name) printf("%s: %c\n", label, name);
    else printf("%s: 없음\n", label);
}

static char at(const ArrayTree *a, int i)
{
    return exists(a, i) ? a->data[i] : '\0';
}

static void arrayQuery(const ArrayTree *a, char key)
{
    int i;
    for (i = 1; i < a->capacity; i++)
        if (a->data[i] == key) break;
    if (i == a->capacity) { puts("노드가 없습니다."); return; }
    relation("왼쪽 자식", at(a, 2 * i));
    relation("오른쪽 자식", at(a, 2 * i + 1));
    relation("부모", at(a, i / 2));
    relation("형제", i == 1 ? '\0' : at(a, i % 2 == 0 ? i + 1 : i - 1));
}

static void showMemory(const char *input)
{
    const char *samples[] = {input, "A(B(D,E),C(F,G(H,)))",
        "A(B(D,E),C(F,G))", "A(B(D,E),C(F,))",
        "A(B(C(D(E(F(G(H(I(J)))))))))"};
    const char *labels[] = {"입력", "일반", "완전/포화", "완전/비포화", "편향"};
    puts("\n[배열 메모리] 종류: 노드 수, 높이, 할당 칸 수, bytes");
    for (int i = 0; i < 5; i++) {
        ArrayTree a = parse(samples[i]);
        Info s = arrayInfo(&a);
        size_t bytes = sizeof(ArrayTree) + (size_t)a.capacity * sizeof(char);
        printf("%s: %d, %d, %d, %zu\n", labels[i], s.nodes,
               s.height, a.capacity, bytes);
        free(a.data);
    }
}

int main(int argc, char *argv[])
{
    char input[1024], key;
    const char *text;
    if (argc > 1) text = argv[1];
    else {
        printf("트리 입력: ");
        if (!fgets(input, sizeof(input), stdin)) return 0;
        if (!strchr(input, '\n') && !feof(stdin)) error("입력이 너무 깁니다.");
        text = input;
    }
    if (strlen(text) >= sizeof(input)) error("입력이 너무 깁니다.");
    ArrayTree a = parse(text);
    if (!exists(&a, 1)) { puts("빈 트리입니다."); free(a.data); return 0; }
    Info result = arrayInfo(&a);
    puts("\n[배열]");
    arrayPrint(&a, 1, 0);
    printInfo(result, arrayComplete(&a, result.nodes));
    showMemory(text);
    printf("\n조회할 노드 (종료: q): ");
    while (scanf(" %c", &key) == 1 && key != 'q') {
        arrayQuery(&a, key);
        printf("\n조회할 노드 (종료: q): ");
    }
    free(a.data);
    return 0;
}
