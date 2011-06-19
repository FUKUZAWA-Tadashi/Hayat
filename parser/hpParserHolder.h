
#ifndef m_PARSERHOLDER_H_
#define m_PARSERHOLDER_H_

namespace Hayat {
    namespace Parser {

        class Parser;
        typedef int ParserID_t;
    
        class ParserHolder {

        public:
            ParserHolder(int maxParser);
            ~ParserHolder();

            ParserID_t entry(Parser* parser);
            Parser* get(ParserID_t id);

        protected:
            int             m_maxParser;
            int             m_numParser;
            Parser**        m_parsers;
        };

    }
}

#endif /* m_PARSERHOLDER_H_ */
