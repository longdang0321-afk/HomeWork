#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEVELS 20

typedef struct Node {
    char name;
    struct Node *left, *right;
} Node;

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

static Node *parseNode(int level)
{
    spaces();
    if (*pos == ',' || *pos == ')') return NULL;
    if (level > MAX_LEVELS) error("최대 20층까지 지원합니다.");
    if (*pos < 'A' || *pos > 'Z') error("노드 이름은 A~Z입니다.");
    if (used[*pos - 'A']) error("이름이 중복됩니다.");
    used[*pos - 'A'] = 1;
    Node *t = malloc(sizeof(Node));
    if (!t) error("메모리 할당 실패");
    t->name = *pos++;
    t->left = t->right = NULL;
    spaces();
    if (*pos == '(') {
        pos++;
        t->left = parseNode(level + 1);
        spaces();
        if (*pos == ',') {
            pos++;
            t->right = parseNode(level + 1);
        }
        spaces();
        if (*pos != ')') error("괄호 또는 자식 수를 확인하세요.");
        pos++;
    }
    return t;
}

static Node *parse(const char *text)
{
    pos = text;
    memset(used, 0, sizeof(used));
    spaces();
    if (!*pos) return NULL;
    Node *root = parseNode(1);
    spaces();
    if (*pos) error("입력 뒤에 불필요한 문자가 있습니다.");
    return root;
}

static void freeTree(Node *t)
{
    if (!t) return;
    freeTree(t->left);
    freeTree(t->right);
    free(t);
}

static Info linkedInfo(Node *t)
{
    Info s = {0, 0, -1, 0, 0, 0};
    if (!t) return s;
    Info l = linkedInfo(t->left), r = linkedInfo(t->right);
    int children = (t->left != NULL) + (t->right != NULL);
    s.nodes = 1 + l.nodes + r.nodes;
    s.leaves = children == 0 ? 1 : l.leaves + r.leaves;
    s.height = 1 + (l.height > r.height ? l.height : r.height);
    s.degree = children;
    if (l.degree > s.degree) s.degree = l.degree;
    if (r.degree > s.degree) s.degree = r.degree;
    s.left = l.left + r.left + (t->left != NULL);
    s.right = l.right + r.right + (t->right != NULL);
    return s;
}

static void linkedPrint(Node *t, int depth)
{
    if (!t) return;
    linkedPrint(t->right, depth + 1);
    printf("%*s%c\n", depth * 4, "", t->name);
    linkedPrint(t->left, depth + 1);
}

static int linkedComplete(Node *t, int i, int n)
{
    if (!t) return 1;
    if (i > n) return 0;
    return linkedComplete(t->left, 2 * i, n)
        && linkedComplete(t->right, 2 * i + 1, n);
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

static Node *find(Node *t, Node *parent, char key, Node **foundParent)
{
    if (!t) return NULL;
    if (t->name == key) { *foundParent = parent; return t; }
    Node *result = find(t->left, t, key, foundParent);
    if (result) return result;
    return find(t->right, t, key, foundParent);
}

static void linkedQuery(Node *root, char key)
{
    Node *parent = NULL, *sibling = NULL;
    Node *t = find(root, NULL, key, &parent);
    if (!t) { puts("노드가 없습니다."); return; }
    if (parent) sibling = parent->left == t ? parent->right : parent->left;
    relation("왼쪽 자식", t->left ? t->left->name : '\0');
    relation("오른쪽 자식", t->right ? t->right->name : '\0');
    relation("부모", parent ? parent->name : '\0');
    relation("형제", sibling ? sibling->name : '\0');
}

static void showMemory(const char *input)
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
               s.height, (size_t)s.nodes * sizeof(Node));
        freeTree(root);
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
    Node *root = parse(text);
    if (!root) { puts("빈 트리입니다."); return 0; }
    Info result = linkedInfo(root);
    puts("\n[연결]");
    linkedPrint(root, 0);
    printInfo(result, linkedComplete(root, 1, result.nodes));
    showMemory(text);
    printf("\n조회할 노드 (종료: q): ");
    while (scanf(" %c", &key) == 1 && key != 'q') {
        linkedQuery(root, key);
        printf("\n조회할 노드 (종료: q): ");
    }
    freeTree(root);
    return 0;
}
