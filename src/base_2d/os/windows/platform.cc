#include <os/platform.h>
#include <iostream>
#include <shobjidl.h>
#include <vector>
#include <string>

// TODO
namespace neptune {

    void debugFunction_01() {
        std::cout << "Hello world!" << std::endl;
    }
    
    void fixFileExplorerPolicy() {
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        if (FAILED(hr)) {
            std::cerr << "Failed to initialize COM library. Error code: " << std::hex << hr << std::endl;
            return;
        }
    }

    std::string WideToUtf8(const wchar_t* wstr)
    {
        if (!wstr) return {};

        int size = WideCharToMultiByte(
            CP_UTF8,
            0,
            wstr,
            -1,
            nullptr,
            0,
            nullptr,
            nullptr
        );

        std::string result(size - 1, 0);

        WideCharToMultiByte(
            CP_UTF8,
            0,
            wstr,
            -1,
            result.data(),
            size,
            nullptr,
            nullptr
        );

        return result;
    }

    std::string getFileFromPicker() {
        // Holy save
        // https://gist.github.com/0xF5T9/3f3203950f480d348aa6d99850a26016
        IFileOpenDialog* pFileOpen;
        COMDLG_FILTERSPEC fileTypes[] = {
            { L"Neptune Project Files", L"*.project;*.zip" },
            { L"All Files", L"*.*" }
        };
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void **>(&pFileOpen));
        if (FAILED(hr)) {
            std::cerr << "Failed to create File Open Dialog instance. Error code: " << std::hex << hr << std::endl;
            return nullptr;
        }
        hr = pFileOpen->SetFileTypes(ARRAYSIZE(fileTypes), fileTypes);
        if (FAILED(hr)) {
            std::cerr << "Failed to set file types for File Open Dialog. Error code: " << std::hex << hr << std::endl;
            pFileOpen->Release();
            return nullptr;
        }
        hr = pFileOpen->Show(NULL);
        if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
        {
            pFileOpen->Release();
            return nullptr;
        }
        else if (FAILED(hr)) {
            std::cerr << "Failed to show File Open Dialog. Error code: " << std::hex << hr << std::endl;
            pFileOpen->Release();
            return nullptr;
        }
        IShellItemArray* pItemArray;
        hr = pFileOpen->GetResults(&pItemArray);
        if (FAILED(hr)) {
            std::cerr << "Failed to get results from File Open Dialog. Error code: " << std::hex << hr << std::endl;
            pFileOpen->Release();
            return nullptr;
        }
        DWORD numItems;
        hr = pItemArray->GetCount(&numItems);
        if (FAILED(hr) || numItems == 0) {
            std::cerr << "Failed to get item count from File Open Dialog results. Error code: " << std::hex << hr << std::endl;
            pItemArray->Release();
            pFileOpen->Release();
            return nullptr;
        }
        IShellItem* pItem;
        hr = pItemArray->GetItemAt(0, &pItem);
        if (FAILED(hr)) {
            std::cerr << "Failed to get item from File Open Dialog results. Error code: " << std::hex << hr << std::endl;
            pItemArray->Release();
            pFileOpen->Release();
            return nullptr;
        }
        PWSTR pszFilePath;
        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
        if (FAILED(hr)) {
            std::cerr << "Failed to get file path from selected item. Error code: " << std::hex << hr << std::endl;
            pItem->Release();
            pItemArray->Release();
            pFileOpen->Release();
            return nullptr;
        }
        std::string filePath = WideToUtf8(pszFilePath);
        CoTaskMemFree(pszFilePath);
        return filePath;
    }

    int popUpWindow(const char* title, const char* message) { 
        int msgboxID = MessageBoxA(
            NULL,
            message,
            title,
            MB_ICONEXCLAMATION | MB_OKCANCEL
        );
        return msgboxID;
    }
}