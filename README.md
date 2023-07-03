# Marquee-pp
![image](https://github.com/creeper531100/Marquee-pp/blob/master/image/title.png)

8字顯示器模擬器  

這是一個模擬256x32文字顯示器的程式。它可以模擬一個簡單的8字顯示器界面。  
請注意，這不是一個開箱即用的程式，需要一定的技術去編譯和運行。  

## 使用方法
### font test  
```font test 是用於將 TrueType 字體（ttf）文件轉換為二進制檔案的工具。這個二進制檔案可以被用作跑馬燈文字在顯示器上顯示。```
  
### [show](https://github.com/creeper531100/Marquee-pp/blob/master/show/show.cpp)  
```show 主要的跑馬燈程式庫。它包含了顯示器的顯示內容的設定，以及跑馬燈程式的運行。```

### 安裝依賴庫  

在編譯和運行本程式之前，請確保您的系統已經安裝了以下依賴庫：

- ft2build
- curl
- nlohmann json

# 個人化設定 Setting
若你想對路線或是字型的路徑進行設定，你可以建立`setting.json`進行設定  
```javascript
{
	"font_path": "mingliu7.03/mingliu_Fixedsys_Excelsior.bin", // 字型路徑
	"client_id": "xxxxxxxx-xxxxxxx-xxxx-xxxx", //你的 TDX 帳號
	"client_secret": "xxxxxxxx-xxxxxxx-xxxx-xxxx", //你的 TDX 密碼
	"url" : "https://tdx.transportdata.tw/api/basic/v2/Bus/RealTimeNearStop/City/Taichung/?%24format=JSON" //你要連線的公車路線，詳細請參照 https://tdx.transportdata.tw/
}
```
在命令行中執行以下指令來編譯程式  
請根據您的系統和編譯環境，選擇合適的編譯指令。

非常感謝您的支持，若如果您在使用過程中遇到任何問題，請隨時與我聯絡:P    

- 在項目的 GitHub 存儲庫中提交問題：[GitHub 存儲庫連結](https://github.com/creeper531100/Marquee-pp/issues)
- 提交程式錯誤報告
- 建議新功能或改進的想法
- 提交程式修正或優化的程式碼




