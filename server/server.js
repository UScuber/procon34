const fs = require("fs");
const express = require("express");

const PORT = process.env.PORT || 5000;
const app = express();



app.get("/", (req, res) => {
  res.writeHead(200, { "Content-Type": "text/html" });
  const token = req.query?.token;
  res.end("OK");
});

app.listen(PORT);
console.log(`Server running at ${PORT}`);
