
#include "hpParserHolder.h"
#include "hpParser.h"


using namespace Hayat::Parser;


ParserHolder::ParserHolder(int maxParser)
{
    m_maxParser = maxParser;
    m_numParser = 0;
    m_parsers = gMemPool->allocT<Parser*>(maxParser, "pHol");
}

ParserHolder::~ParserHolder()
{
#if 0
    printf("m_numParser was %d\n", m_numParser);
#endif
    for (int i = m_numParser - 1; i >= 0; --i)
        delete m_parsers[i];
    gMemPool->free(m_parsers);
}


ParserID_t ParserHolder::entry(Parser* parser)
{
    HMD_ASSERT(m_numParser < m_maxParser);
    m_parsers[m_numParser] = parser;
    return (ParserID_t) m_numParser++;
}

Parser* ParserHolder::get(ParserID_t id)
{
    HMD_ASSERT(id < m_numParser);
    return m_parsers[id];
}
