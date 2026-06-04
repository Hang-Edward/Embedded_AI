const http = require("http");
const fs = require("fs");
const path = require("path");

// 中文注释：这个小服务只用于本地预览静态 Demo，不连接真实硬件。
const root = __dirname;
const port = Number(process.env.PORT || 8765);
const mimeTypes = {
  ".html": "text/html;charset=utf-8",
  ".css": "text/css;charset=utf-8",
  ".js": "text/javascript;charset=utf-8",
  ".md": "text/plain;charset=utf-8",
  ".svg": "image/svg+xml;charset=utf-8",
  ".png": "image/png",
  ".pdf": "application/pdf"
};

const server = http.createServer((request, response) => {
  let route = decodeURIComponent(request.url.split("?")[0]);
  if (route === "/") {
    response.writeHead(302, { Location: "/src/web/index.html" });
    response.end();
    return;
  }

  const filePath = path.normalize(path.join(root, route));
  if (!filePath.startsWith(root)) {
    response.writeHead(403);
    response.end("forbidden");
    return;
  }

  fs.readFile(filePath, (error, data) => {
    if (error) {
      response.writeHead(404);
      response.end("not found");
      return;
    }

    response.writeHead(200, {
      "Content-Type": mimeTypes[path.extname(filePath)] || "application/octet-stream"
    });
    response.end(data);
  });
});

server.listen(port, "127.0.0.1", () => {
  console.log(`Embedded AI Demo: http://127.0.0.1:${port}/`);
});
