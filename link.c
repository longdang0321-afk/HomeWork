#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEVELS 20  /* 최대 층 수: 루트 높이 0, 최대 높이 19 */

typedef struct Node {
    char name;  /* 노드 이름: A~Z 한 글자 */
    struct Node *left, *right;  /* 왼쪽, 오른쪽 자식이 없으면 NULL */
} Node;

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

/* 노드를 만들고 괄호 안의 두 자식을 재귀적으로 연결한다. */
Node *parseNode(int level)
{
    spaces();
    /* 구분자를 바로 만나면 해당 자식 자리는 비어 있다. */
    if (*pos == ',' || *pos == ')') return NULL;
    if (level > MAX_LEVELS) error("최대 20층까지 지원합니다.");
    if (*pos < 'A' || *pos > 'Z') error("노드 이름은 A~Z입니다.");
    if (used[*pos - 'A']) error("이름이 중복됩니다.");
    used[*pos - 'A'] = 1;
    Node *t = malloc(sizeof(Node));  /* 실제 노드 하나의 공간만 할당 */
    if (!t) error("메모리 할당 실패");
    t->name = *pos++;
    t->left = t->right = NULL;
    spaces();
    /* 괄호 안에서 콤마 앞은 왼쪽 자식, 뒤는 오른쪽 자식이다. */
    if (*pos == '(') {
        pos++;
        t->left = parseNode(level + 1);  /* 왼쪽 부분 트리 연결 */
        spaces();
        if (*pos == ',') {
            pos++;
            t->right = parseNode(level + 1);  /* 오른쪽 부분 트리 연결 */
        }
        spaces();
        if (*pos != ')') error("괄호 또는 자식 수를 확인하세요.");
        pos++;
    }
    return t;
}

/* 입력 전체를 읽고 완성된 트리의 루트 포인터를 반환한다. */
Node *parse(const char *text)
{
    pos = text;
    memset(used, 0, sizeof(used));  /* 새 트리를 읽기 전에 이름 기록 초기화 */
    spaces();
    if (!*pos) return NULL;
    Node *root = parseNode(1);
    spaces();
    if (*pos) error("입력 뒤에 불필요한 문자가 있습니다.");
    return root;
}

/* 자식을 먼저 해제한 다음 현재 노드를 해제한다. */
void freeTree(Node *t)
{
    if (!t) return;
    freeTree(t->left);
    freeTree(t->right);
    free(t);
}

/* 왼쪽과 오른쪽 부분 트리의 정보를 합쳐 현재 트리 정보를 구한다. */
Info linkedInfo(Node *t)
{
    Info s = {0, 0, -1, 0, 0, 0};  /* 빈 트리는 높이 -1 */
    if (!t) return s;
    Info l = linkedInfo(t->left), r = linkedInfo(t->right);
    int children = (t->left != NULL) + (t->right != NULL);
    s.nodes = 1 + l.nodes + r.nodes;
    s.leaves = children == 0 ? 1 : l.leaves + r.leaves;  /* 자식이 없으면 단말 */
    /* 두 부분 트리 중 더 높은 쪽에 현재 층 하나를 더한다. */
    s.height = 1 + (l.height > r.height ? l.height : r.height);
    s.degree = children;
    if (l.degree > s.degree) s.degree = l.degree;
    if (r.degree > s.degree) s.degree = r.degree;
    s.left = l.left + r.left + (t->left != NULL);
    s.right = l.right + r.right + (t->right != NULL);
    return s;
}

/* 오른쪽 → 현재 노드 → 왼쪽 순서로, 깊이만큼 들여쓰기한다. */
void linkedPrint(Node *t, int depth)
{
    if (!t) return;
    linkedPrint(t->right, depth + 1);
    printf("%*s%c\n", depth * 4, "", t->name);
    linkedPrint(t->left, depth + 1);
}

/* 가상 인덱스를 붙여 실제 노드가 1~n번 범위를 벗어나는지 확인한다. */
int linkedComplete(Node *t, int i, int n)
{
    if (!t) return 1;
    if (i > n) return 0;
    return linkedComplete(t->left, 2 * i, n)
        && linkedComplete(t->right, 2 * i + 1, n);
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

/* 이름 검색 중 부모도 함께 전달하여 부모를 다시 찾지 않도록 한다. */
Node *find(Node *t, Node *parent, char key, Node **foundParent)
{
    if (!t) return NULL;
    /* 찾은 노드의 부모를 호출한 함수의 변수에 저장한다. */
    if (t->name == key) { *foundParent = parent; return t; }
    Node *result = find(t->left, t, key, foundParent);
    if (result) return result;
    return find(t->right, t, key, foundParent);
}

/* 찾은 노드와 부모 포인터를 사용해 관계를 출력한다. */
void linkedQuery(Node *root, char key)
{
    Node *parent = NULL, *sibling = NULL;
    Node *t = find(root, NULL, key, &parent);
    if (!t) { puts("노드가 없습니다."); return; }
    /* 같은 부모의 다른 쪽 자식이 형제이다. */
    if (parent) sibling = parent->left == t ? parent->right : parent->left;
    relation("왼쪽 자식", t->left ? t->left->name : '\0');
    relation("오른쪽 자식", t->right ? t->right->name : '\0');
    relation("부모", parent ? parent->name : '\0');
    relation("형제", sibling ? sibling->name : '\0');
}

/* 입력 트리와 공통 예제의 최종 저장 공간을 계산한다. */
void showMemory(const char *input)
{
    const char *samples[] = {input, "A(B(D,E),C(F,G(H,)))",
        "A(B(D,E),C(F,G))", "A(B(D,E),C(F,))",
        "A(B(C(D(E(F(G(H(I(J)))))))))"};
    const char *labels[] = {"입력", "일반", "완전/포화", "완전/비포화", "편향"};
    puts("\n[연결 메모리] 종류: 노드 수, 높이, bytes");
    for (int i = 0; i < 5; i++) {
        Node *root = parse(samples[i]);
        Info s = linkedInfo(root);
        printf("%s: %d, %d, %zu\n", labels[i], s.nodes,
               s.height, (size_t)s.nodes * sizeof(Node));  /* 노드 수 × 노드 크기 */
        freeTree(root);
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
    Node *root = parse(text);
    if (!root) { puts("빈 트리입니다."); return 0; }
    Info result = linkedInfo(root);
    puts("\n[연결]");
    linkedPrint(root, 0);
    printInfo(result, linkedComplete(root, 1, result.nodes));
    showMemory(text);
    printf("\n조회할 노드 (종료: q): ");
    /* 노드 이름을 반복 조회하며, 소문자 q를 입력하면 종료한다. */
    while (scanf(" %c", &key) == 1 && key != 'q') {
        linkedQuery(root, key);
        printf("\n조회할 노드 (종료: q): ");
    }
    freeTree(root);  /* 종료 전에 모든 노드 해제 */
    return 0;
}
