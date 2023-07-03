# Marquee-pp
[![image](https://github.com/creeper531100/Marquee-pp/image/title.png)](https://github.com/creeper531100/Marquee-pp/blob/master/image/title.png)

8字顯示器模擬器  

這是一個模擬256x32文字顯示器的程式。它可以模擬一個簡單的8字顯示器界面。請注意，這不是一個開箱即用的程式，需要一定的技術去編譯和運行。  

## 使用方法
### font test  
```font test 是用於將 TrueType 字體（ttf）文件轉換為二進制檔案的工具。這個二進制檔案可以被用作跑馬燈文字在顯示器上顯示。```
  
### [show](https://github.com/creeper531100/Marquee-pp/blob/master/show/show.cpp) 库  
```show 主要的跑馬燈程式庫。它包含了顯示器的顯示內容的設定，以及跑馬燈程式的運行。```

### 安裝依賴庫  

在編譯和運行本程式之前，請確保您的系統已經安裝了以下庫：

- ft2build
- curl
- nlohmann json

在命令行中執行以下指令來編譯程式  
請根據您的系統和編譯環境，選擇合適的編譯指令。
