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

            static StrTable     m_basicSignatureTable; // 基本シグネチャ情報


        public:
            Package(SymbolID_t sym);
            Package(void); // for deserialize
            ~Package();

            SymbolID_t  getSymbol(void) { return m_mySymbol; }
            void require(SymbolID_t sym);
            void linkPackage(SymbolID_t sym);

            // 自パッケージクラス
            ClassInfo* packageClass(void) { return m_packageClass; }

            //// シグネチャ
            // シグネチャバイト列に対応するシグネチャIDを取得
            // 既存のIDが無ければ新規登録する
            hyu16 getSignatureID(const hyu8* sigBytes, hyu32 len);
            // シグネチャIDから、シグネチャバイト列を取得
            const Signature::Sig_t* getSignatureBytes(hyu16 signatureID);

            // シグネチャテーブルをファイル出力
            void fwriteSignatureTable(FILE* fp);
            // 引数個数情報をファイル出力
            void fwriteArityTable(FILE* fp);

            // リンク情報をファイル出力
            void fwriteLinks(FILE* fp);


            //// シリアライズ
            // ClassInfo登録
            void addClassInfo(ClassInfo* ci) { m_classInfos.add(ci); }
            // ClassInfo番号取得: 該当しなければ -1
            hys32 classInfo2ID(ClassInfo*);
            // パッケージシンボルとClassInfo番号からClassInfoを取得
            static ClassInfo* ID2classInfo(SymbolID_t pkgSym, hys32 classID);
            // ClassInfo番号からClassInfoを取得
            ClassInfo* ID2classInfo(hys32 classID);

            void serialize(TArray<hyu8>* out);
            const hyu8* deserialize(const hyu8* inp);

            void fwritePackage(FILE* fp);
            static const hyu8* readPackage(const hyu8* buf, hyu32 bufSize, const char* fname);


        protected:
            SymbolID_t          m_mySymbol;      // パッケージシンボル
            TArray<SymbolID_t>  m_linkPackages;  // requireしたパッケージ(require順)
            TArray<ClassInfo*>  m_classInfos;    // このパッケージが管理するClassInfo
            StrTable            m_signatureTable;        // シグネチャ情報
            
            ClassInfo*          m_packageClass;




            //// warning/errorチェック用情報
        public:
            // クラスが参照された時に呼び出す
            // そのクラスがあれば何もせず、なければ最終チェックのために記録
            void checkClass(SymbolID_t classSym, hyu32 parsePos);
            // メソッド名が参照された時に呼び出す
            // 対応メソッドがあれば true を返し、なければ最終チェックの
            // ために記録しておき、false を返す
            bool checkMethod(SymbolID_t methodSym, hyu16 signatureID, hyu32 parsePos);
            // 同名メソッドがパッケージから呼び出せる場所にあるかチェック
            // メソッド名のみチェックしてシグネチャは見ない
            bool checkMethodName(SymbolID_t methodSym) {
                return m_packageClass->checkMethodName(methodSym); }
            // 不明なクラス、メソッドを最終チェックしてwarning/error出力
            // checkLevel: <=0 → no check , ==1 → warning , >=2 → error
            bool finalCheck(int checkLevel);
        protected:
            // 既知のクラス
            TArray<SymbolID_t>          m_knownClasses;
            // checkClass()時点で不明なクラス
            BMap<SymbolID_t, hyu32>       m_unknownClasses;
            // checkMethod()時点で不明なメソッド
            typedef struct { hyu32 pos; hyu16 signatureID; } UM_t;
            BMap<SymbolID_t, UM_t>       m_unknownMethods;

        };

    }
}

#endif /* m_HYCPACKAGE_H_ */
