#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEVELS 20  /* 최대 층 수: 루트 높이 0, 최대 높이 19 */

typedef struct {
    char *data;  /* 노드 이름 배열: 루트는 1번, 빈 칸은 0 */
    int capacity;  /* 사용하지 않는 0번을 포함한 전체 할당 칸 수 */
} ArrayTree;

typedef struct {
    int nodes, leaves, height, degree, left, right;
    /* 전체 노드, 단말, 높이, 차수, 왼쪽 연결 수, 오른쪽 연결 수 */
} Info;

const char *pos;  /* 입력 문자열에서 현재 읽는 위치 */
int used[26];     /* A~Z 이름의 중복 사용 확인 */

void error(const char *message)
{
    fprintf(stderr, "오류: %s\n", message);
    exit(1);
}

/* 이름과 괄호 사이의 공백, 탭, 줄바꿈을 건너뛴다. */
void spaces(void)
{
    while (isspace((unsigned char)*pos)) pos++;
}

/* 괄호를 읽으며 인덱스 위치에 직접 저장한다. 연결 노드는 만들지 않는다. */
void parseNode(ArrayTree *a, int index, int level)
{
    spaces();
    /* 구분자를 바로 만나면 해당 자식 자리는 비어 있다. */
    if (*pos == ',' || *pos == ')') return;
    if (level > MAX_LEVELS) error("최대 20층까지 지원합니다.");
    if (*pos < 'A' || *pos > 'Z') error("노드 이름은 A~Z입니다.");
    if (used[*pos - 'A']) error("이름이 중복됩니다.");
    used[*pos - 'A'] = 1;
    /* 필요한 인덱스를 담을 수 있을 때까지 배열 크기를 두 배로 늘린다. */
    while (index >= a->capacity) {
        int newCapacity = a->capacity * 2;
        char *temp = realloc(a->data, (size_t)newCapacity);
        if (!temp) error("메모리 할당 실패");
        /* 새로 생긴 칸은 아직 노드가 없으므로 0으로 초기화한다. */
        memset(temp + a->capacity, 0, (size_t)(newCapacity - a->capacity));
        a->data = temp;
        a->capacity = newCapacity;
    }
    a->data[index] = *pos++;
    spaces();
    /* 괄호 안에서 콤마 앞은 왼쪽 자식, 뒤는 오른쪽 자식이다. */
    if (*pos == '(') {
        pos++;
        parseNode(a, 2 * index, level + 1);  /* 왼쪽 자식 위치 */
        spaces();
        if (*pos == ',') {
            pos++;
            parseNode(a, 2 * index + 1, level + 1);  /* 오른쪽 자식 위치 */
        }
        spaces();
        if (*pos != ')') error("괄호 또는 자식 수를 확인하세요.");
        pos++;
    }
}

/* 루트 인덱스 1부터 입력을 읽어 배열 트리를 만든다. */
ArrayTree parse(const char *text)
{
    ArrayTree a = {calloc(2, sizeof(char)), 2};
    if (!a.data) error("메모리 할당 실패");
    pos = text;
    memset(used, 0, sizeof(used));  /* 새 트리를 읽기 전에 이름 기록 초기화 */
    spaces();
    if (*pos) parseNode(&a, 1, 1);
    spaces();
    if (*pos) error("입력 뒤에 불필요한 문자가 있습니다.");
    return a;
}

/* 배열 범위 안에 실제 노드가 있는지 확인한다. */
int exists(const ArrayTree *a, int i)
{
    return i > 0 && i < a->capacity && a->data[i] != '\0';
}

/* 빈 칸을 제외하고 노드 수, 단말 수, 높이, 차수를 구한다. */
Info arrayInfo(const ArrayTree *a)
{
    Info s = {0, 0, -1, 0, 0, 0};  /* 빈 트리는 높이 -1 */
    for (int i = 1; i < a->capacity; i++) {
        if (!exists(a, i)) continue;
        int l = exists(a, 2 * i), r = exists(a, 2 * i + 1);
        int depth = 0;
        /* 부모 인덱스로 올라간 횟수가 현재 노드의 깊이이다. */
        for (int j = i; j > 1; j /= 2) depth++;
        s.nodes++;
        s.leaves += (l + r == 0);  /* 자식이 없으면 단말 노드 */
        s.left += l;
        s.right += r;
        if (depth > s.height) s.height = depth;
        if (l + r > s.degree) s.degree = l + r;
    }
    return s;
}

/* 오른쪽 → 현재 노드 → 왼쪽 순서로, 깊이만큼 들여쓰기한다. */
void arrayPrint(const ArrayTree *a, int i, int depth)
{
    if (!exists(a, i)) return;
    arrayPrint(a, 2 * i + 1, depth + 1);
    printf("%*s%c\n", depth * 4, "", a->data[i]);
    arrayPrint(a, 2 * i, depth + 1);
}

/* 노드가 n개일 때 1~n번 위치가 모두 차 있으면 완전 이진트리이다. */
int arrayComplete(const ArrayTree *a, int n)
{
    for (int i = 1; i <= n; i++)
        if (!exists(a, i)) return 0;
    return 1;
}

/* 계산한 정보와 세 가지 트리 형태를 출력한다. */
void printInfo(Info s, int complete)
{
    /* 높이 h의 모든 층이 차 있으면 노드 수는 2^(h+1)-1이다. */
    int perfect = s.nodes == (1 << (s.height + 1)) - 1;
    /* 한 줄 구조이며 연결 방향이 한쪽뿐이면 편향으로 판별한다. */
    int skew = s.nodes == s.height + 1 && (s.left == 0 || s.right == 0);
    printf("전체 노드: %d / 단말: %d / 비단말: %d\n",
           s.nodes, s.leaves, s.nodes - s.leaves);
    printf("높이: %d / 차수: %d\n", s.height, s.degree);
    printf("완전: %s / 포화: %s / 편향: %s\n",
           complete ? "예" : "아니오", perfect ? "예" : "아니오",
           skew ? "예" : "아니오");
}

/* 관계에 해당하는 노드가 없으면 "없음"으로 표시한다. */
void relation(const char *label, char name)
{
    if (name) printf("%s: %c\n", label, name);
    else printf("%s: 없음\n", label);
}

/* 해당 위치의 이름을 반환하고, 노드가 없으면 0을 반환한다. */
char at(const ArrayTree *a, int i)
{
    return exists(a, i) ? a->data[i] : '\0';
}

/* 이름을 찾은 뒤 인덱스 계산으로 자식, 부모, 형제를 구한다. */
void arrayQuery(const ArrayTree *a, char key)
{
    int i;
    for (i = 1; i < a->capacity; i++)
        if (a->data[i] == key) break;
    if (i == a->capacity) { puts("노드가 없습니다."); return; }
    relation("왼쪽 자식", at(a, 2 * i));
    relation("오른쪽 자식", at(a, 2 * i + 1));
    relation("부모", at(a, i / 2));
    /* 루트에는 형제가 없다. 짝수 위치는 +1, 홀수 위치는 -1이다. */
    relation("형제", i == 1 ? '\0' : at(a, i % 2 == 0 ? i + 1 : i - 1));
}

/* 입력 트리와 공통 예제의 최종 저장 공간을 계산한다. */
void showMemory(const char *input)
{
    const char *samples[] = {input, "A(B(D,E),C(F,G(H,)))",
        "A(B(D,E),C(F,G))", "A(B(D,E),C(F,))",
        "A(B(C(D(E(F(G(H(I(J)))))))))"};
    const char *labels[] = {"입력", "일반", "완전/포화", "완전/비포화", "편향"};
    puts("\n[배열 메모리] 종류: 노드 수, 높이, 할당 칸 수, bytes");
    for (int i = 0; i < 5; i++) {
        ArrayTree a = parse(samples[i]);
        Info s = arrayInfo(&a);
        /* 관리 구조체와 빈 칸을 포함한 전체 배열 공간을 합산한다. */
        size_t bytes = sizeof(ArrayTree) + (size_t)a.capacity * sizeof(char);
        printf("%s: %d, %d, %d, %zu\n", labels[i], s.nodes,
               s.height, a.capacity, bytes);
        free(a.data);
    }
}

/* 입력 → 트리 출력 및 정보 확인 → 메모리 확인 → 노드 조회 */
int main(int argc, char *argv[])
{
    char input[1024], key;
    const char *text;
    if (argc > 1) text = argv[1];  /* 실행 인자가 없으면 아래에서 직접 입력 */
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
    /* 노드 이름을 반복 조회하며, 소문자 q를 입력하면 종료한다. */
    while (scanf(" %c", &key) == 1 && key != 'q') {
        arrayQuery(&a, key);
        printf("\n조회할 노드 (종료: q): ");
    }
    free(a.data);  /* 종료 전에 배열 메모리 해제 */
    return 0;
}
