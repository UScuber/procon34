const fs = require("fs");
const express = require("express");
const { exec, execSync } = require("child_process");
const axios = require("axios");

const PORT = process.env.PORT || 5000;
const app = express();

const run_command = (cmd) => {
  return execSync(cmd).toString();
};
const read_file = (path) => {
  if(fs.existsSync(path)) return fs.readFileSync(path).toString();
  return "";
};

const match_id = 10;
const match_data = require("./match.json");
console.log("match data:", match_data);
const URL = `http://localhost:3000/matches/${match_id}?token=randomagent`;
let is_running = false;
let isnot_first = true;
let clean_func = undefined;

const execute_randomagent = async(match_data, current_turn) => {
  let str = "";
  const board = match_data.board;
  str += `${board.height} ${board.width} ${board.mason} ${current_turn}\n`;
  // walls
  for(let i = 0; i < board.height; i++){
    for(let j = 0; j < board.width; j++){
      str += board.walls[i][j] + " ";
    }
    str += "\n";
  }
  // territories
  for(let i = 0; i < board.height; i++){
    for(let j = 0; j < board.width; j++){
      str += board.territories[i][j] + " ";
    }
    str += "\n";
  }
  // masons
  for(let i = 0; i < board.height; i++){
    for(let j = 0; j < board.width; j++){
      str += board.masons[i][j] + " ";
    }
    str += "\n";
  }
  fs.writeFileSync("input.txt", str);
  execSync("./programs/randomagent.exe < input.txt > output.txt", (err, stdout, stderr) => {
    console.log("executed:", err, stdout, stderr);
  });
  const output = read_file("output.txt");
  console.log("output:", output);
  const json = JSON.parse(output);
  console.log("json:", json);
  await axios.post(URL, json)
    .then((res) => console.log("post res:", res))
    .catch(console.error);
};

const launch_random_agent = () => {
  let last_log_length = -1;
  const interval_id = setInterval(async() => {
    if(!is_running) clearInterval(interval_id);
    try {
      const response = await axios.get(URL);
      if(response.data.data === "TooEarly") return;
      const data = response.data;
      if(data.logs.length !== last_log_length){
        last_log_length = data.logs.length;
        console.log("Response data:", response.data);
        // random agent's turn
        if((last_log_length & 1) == isnot_first){
          console.log("last log length:", last_log_length);
          await execute_randomagent(data, last_log_length);
        }
      }
    }catch(err){
      console.error(err.response);
    }
  }, 100);
};



app.get("/", (req, res) => {
  res.writeHead(200, { "Content-Type": "text/html" });
  res.end("OK");
});


app.get("/start", async(req, res) => {
  res.writeHead(200, { "Content-Type": "text/html" });
  if(is_running){
    console.log("Server is already running");
    res.end("Server is already running");
    return;
  }
  is_running = true;
  console.log("Start Launch Server");
  res.end("Start Launch Server");
  const cp = exec("./procon-server_linux -c match.json", (err, stdout, stderr) => {
    if(err){
      console.error("Error:", stderr);
      return;
    }
    console.log("Out:", stdout);
  });
  console.log("first:", match_data.first);
  // isnot_first = !match_data.first;
  launch_random_agent();
  console.log("pid:", cp.pid);
  clean_func = () => { process.kill(cp.pid); };
});

app.get("/stop", (req, res) => {
  res.writeHead(200, { "Content-Type": "text/html" });
  if(!is_running){
    console.log("Server is already stopped");
    res.end("Server is already stopped");
    return;
  }
  is_running = false;
  console.log("Stop Server");
  res.end("Stop Server");
  clean_func();
});


app.listen(PORT);
console.log(`Server running at ${PORT}`);
