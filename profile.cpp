
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
 
 
 
/*
    ini�t�@�C���֏�������
*/
int _tmain
(
      int argc
    , _TCHAR* argv[]
)
{
    /*
        std::wcout�̃��P�[����ݒ�
        �@�����ݒ肷�邾���ŁAstd::wcout�œ��{�ꂪ�\�������
        �@�悤�ɂȂ�܂��B
    */
    std::wcout.imbue( std::locale( "", std::locale::ctype ) );
 
    // ini�t�@�C���p�X
    std::wstring strInfilePath = L".\\TestUnicode.ini";
 char *q=".\\TestUnicode.ini";
 
    /*
        ini�t�@�C����������
    */
    if (!WritePrivateProfileString( 
  "SECTION", "key_str", "�e�X�g������", q ) ){
 
        std::wcout << L"ini�t�@�C���ւ̏������݂Ɏ��s���܂����B" << std::endl;
    }
    else {
 
        std::wcout << L"ini�t�@�C���֏������݂܂����B" << std::endl;
    }
 
 
    /*
        ini�t�@�C����������(���l)
     �ǂݏo���ł͂��AGetProfileInt()�����݂��܂����AWritePrivateProfileInt��
        ���݂��Ȃ��̂ŁA���l�͕����񉻂��ď������݂܂��B
    */
    if ( 0 == ::WritePrivateProfileString( "SECTION", "key_value", "1234",
     q ) ) {
 
        std::wcout << L"ini�t�@�C���ւ̏������݂Ɏ��s���܂����B" << std::endl;
    }
    else {
 
        std::wcout << L"ini�t�@�C���֏������݂܂����B" << std::endl;
    }
 
    // ����I��
    return( 0 );
}
