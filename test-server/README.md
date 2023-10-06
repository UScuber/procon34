# test-server

visualizerの動作確認用に作成したテスト用のサーバーです


## 初期設定

```
npm i

g++ programs/main.cpp -o programs/randomagent.exe
```

- 対戦に必要なJSON(match.json)の作成

- 簡易サーバーのプログラム(procon-server_linux)の作成

  - ダウンロードした簡易サーバーの実行ファイルは権限がないため、`chomd +x procon-server_linux` を実行して実行できるようにしてください


## 起動方法

```
node server.js
```

## 使い方

- localhost:5000/startにアクセスし、試合を開始する

- localhost:3000に簡易サーバーが立ち上がるため、そこでランダムエージェントとの対戦を行う
