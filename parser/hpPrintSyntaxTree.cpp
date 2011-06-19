
#include "hpPrintSyntaxTree.h"


using namespace Hayat::Parser;


void printSyntaxTree(SyntaxTree* tree, InputBuffer* inp, int indent)
{
    char is[128];
    char buf[128];
    if (tree == NULL) {
        printf("(NULL)\n");
        return;
    }

    if (indent > 60) indent = 60;
    for (int i = indent; --i >= 0;) is[i] = ' ';
    is[indent] = '\0';
    hyu32 len = tree->str.len();
    if ((int)len > 76 - indent)
        len = 76 - indent;
    inp->copySummary(buf, len+1, tree->str);
    if (tree->isErrorCut())
        HMD_PRINTF("{ErrorCut %d}[%s]", tree->errorCutId, buf);
    else if (tree->isFatalError())
        HMD_PRINTF("{FatalError}");
    else if (tree->isError())
        HMD_PRINTF("{Error %d}", tree->errorCutId);
    else if (tree->isFail())
        HMD_PRINTF("{Fail}");
    else if (! tree->isValidTree())
        HMD_PRINTF("{Invalid}");
    else if (tree->chooseNum >= 0)
        HMD_PRINTF("%s %s(%d)[%s]", is, tree->parser->name(), tree->chooseNum, buf);
    else
        HMD_PRINTF("%s %s[%s]", is, tree->parser->name(), buf);
    if ((tree->childs != NULL) && (tree->numChild() > 0)) {
        HMD_PRINTF("(\n");
        for (int i = 0; i < tree->numChild(); i++) {
            printSyntaxTree(tree->get(i), inp, indent+1);
        }
        HMD_PRINTF("%s)\n", is);
    } else {
        HMD_PRINTF("\n");
    }
}
