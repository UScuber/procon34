# test-server

visualizerの動作確認用に作成したテスト用のサーバーです


## 初期設定

```
npm i

g++ programs/main.cpp -o programs/randomagent.exe
```

- 対戦に必要なJSON(match.json)の作成

- 簡易サーバーのプログラム(procon-server_linux)の作成


## 起動方法

```
node server.js
```

## 使い方

- localhost:5000/startにアクセスし、試合を開始する

- localhost:5001に簡易サーバーが立ち上がるため、そこでランダムエージェントとの対戦を行う
