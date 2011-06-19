/*  -*- coding: sjis-dos; -*-  */

/*
 * copyright 2010 FUKUZAWA Tadashi.  All rights reserved.
 */

#ifndef m_HYCPACKAGE_H_
#define m_HYCPACKAGE_H_

#include "hySymbolID.h"
#include "hyBMap.h"
#include "hyCClassInfo.h"
#include "hyCSignature.h"
#include <stdio.h>

class Test_hyCPackage;

namespace Hayat {
    namespace Compiler {

        class Package {

            friend class ::Test_hyCPackage;       // for unittest
            
        public:
            static void initialize(void);
            static void registration(Package* pkg);
            static Package* getPackage(SymbolID_t sym);
            static void finalize(void);


            static void* operator new(size_t size);
            static void operator delete(void* p);

        protected:
            static TArray<Package*>     m_loadedPackages;

            static StrTable     m_basicSignatureTable; // ��{�V�O�l�`�����


        public:
            Package(SymbolID_t sym);
            Package(void); // for deserialize
            ~Package();

            SymbolID_t  getSymbol(void) { return m_mySymbol; }
            void require(SymbolID_t sym);
            void linkPackage(SymbolID_t sym);

            // ���p�b�P�[�W�N���X
            ClassInfo* packageClass(void) { return m_packageClass; }

            //// �V�O�l�`��
            // �V�O�l�`���o�C�g��ɑΉ�����V�O�l�`��ID���擾
            // ������ID��������ΐV�K�o�^����
            hyu16 getSignatureID(const hyu8* sigBytes, hyu32 len);
            // �V�O�l�`��ID����A�V�O�l�`���o�C�g����擾
            const Signature::Sig_t* getSignatureBytes(hyu16 signatureID);

            // �V�O�l�`���e�[�u�����t�@�C���o��
            void fwriteSignatureTable(FILE* fp);
            // �����������t�@�C���o��
            void fwriteArityTable(FILE* fp);

            // �����N�����t�@�C���o��
            void fwriteLinks(FILE* fp);


            //// �V���A���C�Y
            // ClassInfo�o�^
            void addClassInfo(ClassInfo* ci) { m_classInfos.add(ci); }
            // ClassInfo�ԍ��擾: �Y�����Ȃ���� -1
            hys32 classInfo2ID(ClassInfo*);
            // �p�b�P�[�W�V���{����ClassInfo�ԍ�����ClassInfo���擾
            static ClassInfo* ID2classInfo(SymbolID_t pkgSym, hys32 classID);
            // ClassInfo�ԍ�����ClassInfo���擾
            ClassInfo* ID2classInfo(hys32 classID);

            void serialize(TArray<hyu8>* out);
            const hyu8* deserialize(const hyu8* inp);

            void fwritePackage(FILE* fp);
            static const hyu8* readPackage(const hyu8* buf, hyu32 bufSize, const char* fname);


        protected:
            SymbolID_t          m_mySymbol;      // �p�b�P�[�W�V���{��
            TArray<SymbolID_t>  m_linkPackages;  // require�����p�b�P�[�W(require��)
            TArray<ClassInfo*>  m_classInfos;    // ���̃p�b�P�[�W���Ǘ�����ClassInfo
            StrTable            m_signatureTable;        // �V�O�l�`�����
            
            ClassInfo*          m_packageClass;




            //// warning/error�`�F�b�N�p���
        public:
            // �N���X���Q�Ƃ��ꂽ���ɌĂяo��
            // ���̃N���X������Ή��������A�Ȃ���΍ŏI�`�F�b�N�̂��߂ɋL�^
            void checkClass(SymbolID_t classSym, hyu32 parsePos);
            // ���\�b�h�����Q�Ƃ��ꂽ���ɌĂяo��
            // �Ή����\�b�h������� true ��Ԃ��A�Ȃ���΍ŏI�`�F�b�N��
            // ���߂ɋL�^���Ă����Afalse ��Ԃ�
            bool checkMethod(SymbolID_t methodSym, hyu16 signatureID, hyu32 parsePos);
            // �������\�b�h���p�b�P�[�W����Ăяo����ꏊ�ɂ��邩�`�F�b�N
            // ���\�b�h���̂݃`�F�b�N���ăV�O�l�`���͌��Ȃ�
            bool checkMethodName(SymbolID_t methodSym) {
                return m_packageClass->checkMethodName(methodSym); }
            // �s���ȃN���X�A���\�b�h���ŏI�`�F�b�N����warning/error�o��
            // checkLevel: <=0 �� no check , ==1 �� warning , >=2 �� error
            bool finalCheck(int checkLevel);
        protected:
            // ���m�̃N���X
            TArray<SymbolID_t>          m_knownClasses;
            // checkClass()���_�ŕs���ȃN���X
            BMap<SymbolID_t, hyu32>       m_unknownClasses;
            // checkMethod()���_�ŕs���ȃ��\�b�h
            typedef struct { hyu32 pos; hyu16 signatureID; } UM_t;
            BMap<SymbolID_t, UM_t>       m_unknownMethods;

        };

    }
}

#endif /* m_HYCPACKAGE_H_ */
