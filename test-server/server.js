const fs = require("fs");
const express = require("express");
const { exec, execSync } = require("child_process");
const axios = require("axios");

const PORT = process.env.PORT || 5000;
const app = express();

const read_file = (path) => {
  if(fs.existsSync(path)) return fs.readFileSync(path).toString();
  return "";
};

const match_id = 10;
const match_data = require("./match.json");
console.log("match data:", match_data);
const URL = `http://localhost:3000/matches/${match_id}?token=randomagent`;
const is_windows = process.platform === "win32";
const is_mac = process.platform === "darwin";
let is_running = false;
let isnot_first = true;
let clean_func = undefined;

const execute_randomagent = async(match_data, current_turn) => {
  let str = "";
  const board = match_data.board;
  str += `${board.height} ${board.width} ${board.mason} ${current_turn} ${isnot_first ? 1 : 0}\n`;
  // structures
  for(let i = 0; i < board.height; i++){
    for(let j = 0; j < board.width; j++){
      str += board.structures[i][j] + " ";
    }
    str += "\n";
  }
  str += "\n";
  // walls
  for(let i = 0; i < board.height; i++){
    for(let j = 0; j < board.width; j++){
      str += board.walls[i][j] + " ";
    }
    str += "\n";
  }
  str += "\n";
  // territories
  for(let i = 0; i < board.height; i++){
    for(let j = 0; j < board.width; j++){
      str += board.territories[i][j] + " ";
    }
    str += "\n";
  }
  str += "\n";
  // masons
  for(let i = 0; i < board.height; i++){
    for(let j = 0; j < board.width; j++){
      str += board.masons[i][j] + " ";
    }
    str += "\n";
  }
  
  fs.writeFileSync("input.txt", str);
  if(is_windows) execSync('"programs/randomagent.exe" < input.txt > output.txt');
  else execSync("./programs/randomagent.exe < input.txt > output.txt");
  const output = read_file("output.txt");
  console.log("output:", output);
  const response = await fetch(URL, {
    method: "post",
    body: output,
    headers: { "Content-Type": "application/json" }
  });
  const res_json = await response.json();
  console.log("res:", res_json);
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
        //console.log("Response data:", response.data);
        // random agent's turn
        if((last_log_length & 1) == isnot_first){
          console.log("last log length:", last_log_length);
          console.log("start turn:", last_log_length + 1);
          await execute_randomagent(data, last_log_length + 1);
        }
      }
    }catch(err){
      console.error("Too Early");
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
  console.log("Start Launch Server");
  let filename = "./procon-server_linux";
  if(is_windows) filename = "procon-server_win.exe";
  if(is_mac) filename = "./procon-server_darwin_arm";
  if(!fs.existsSync(filename)){
    console.error("Error: " + filename + " was not found");
    res.end("Error: " + filename + " was not found");
    return;
  }
  const cp = exec(filename + " -c match.json -start 1s", (err, stdout, stderr) => {
    if(err){
      has_error = true;
      return;
    }
    console.log("Out:", stdout);
  });
  res.end("Start Launch Server");
  console.log("first:", match_data.first);
  // isnot_first = !match_data.first;
  launch_random_agent();
  console.log("pid:", cp.pid);
  is_running = true;
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
