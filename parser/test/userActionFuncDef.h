#ifdef m_IN_SYNTAXTREE_H
void action(void);
#endif
#ifdef m_IN_PARSER_H
virtual void action(SyntaxTree*) {}
#endif
