# Web Preview

这里存放静态网页原型，用于展示项目概念、采购清单、硬件方案和答辩路线。它只是展示层，不作为主要 C++ 作业代码。

## 页面

- `index.html`：AI + C++ 嵌入式控制台 Demo。
- `product.html`：AI Reality Bridge 产品概念展示。
- `guide.html`：采购清单、硬件方案、开发步骤和答辩路线。
- `styles.css`：统一页面样式。
- `script.js` / `product.js`：页面交互逻辑。

## 本地预览

从项目根目录运行：

```powershell
node server.js
```

然后访问：

```text
http://127.0.0.1:8765/
```

如果端口被占用：

```powershell
$env:PORT=8766
node server.js
```
