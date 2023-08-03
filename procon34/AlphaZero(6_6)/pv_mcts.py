from game import State, HEIGHT, WIDTH
from dual_network import DN_INPUT_SHAPE
from math import sqrt
from tensorflow.keras.models import load_model
from pathlib import Path
import numpy as np

# パラメータの準備
PV_EVALUATE_COUNT = 50 # 1推論当たりのシミュレーション回数(本家は1600)

# 推論
def predict(model, state:State):
    # 推論のための入力データのシェイプの変換
    a, b, c = DN_INPUT_SHAPE
    x = state.state_array()
    x = x.reshape(c, a, b).transpose(1, 2, 0).reshape(1, a, b, c)

    # 推論
    y = model.predict_on_batch(x)

    # 方策の取得
    policies = y[0][0][state.get_list_of_legal_actions()] # 合法手のみ
    policies /= sum(policies) if sum(policies) else 1 # 合計1の確率分布に変換
    # 価値の取得
    value = y[1][0][0]
    return policies, value

# ノードのリストを試行回数のリストに変換
def nodes_to_scores(nodes):
    scores = []
    for c in nodes:
        scores.append(c.n)
    return scores

# モンテカルロ木探索のスコアの取得
def pv_mcts_scores(model, state:State, temperature):
    # モンテカルロ木探索のノードの定義
    class node:
        def __init__(self, state, p):
            self.state = state # 状態
            self.p = p # 方策
            self.w = 0 # 累計価値
            self.n = 0 # 試行回数
            self.child_nodes = None # 子ノード群
        
        def evaluate(self):
            # ゲーム終了時
            if self.state.is_done():
                # 勝敗結果で価値を取得
                value = -1 if self.state.is_lose() else 0

                # 累計価値と試行回数の更新
                self.w += value
                self.n += 1
                return value
            
            # 子ノードが存在しないとき
            if not self.child_nodes:
                # ニューラルネットワークの推論で方策と価値を取得
                policies, value = predict(model, self.state)

                # 累計価値と試行回数の更新
                self.w += value
                self.n += 1

                # 子ノードの展開
                self.child_nodes = []
                for action, policy in zip(self.state.get_list_of_legal_actions(), policies):
                    self.child_nodes.append(node(self.state.next(action), policy))
                return value
            # 子ノードが存在するとき
            else:
                # アーク評価値が最大の子ノードの評価で価値を取得
                value = -self.next_child_node().evaluate()

                # 累計価値と試行回数の更新
                self.w += value
                self.n += 1
                return value
        
        # アーク評価値が最大の子ノードを取得
        def next_child_node(self):
            # アーク評価値の計算
            C_PUCT = 1.0 # 勝率と手の予測確率*バイアスのバランスを調整するための定数
            t = sum(nodes_to_scores(self.child_nodes))
            pucb_values = []
            for child_node in self.child_nodes:
                pucb_values.append((-child_node.w / child_node.n if child_node.n else 0.0) + C_PUCT * child_node.p * sqrt(t) / (1 + child_node.n))
            # アーク評価値が最大の子ノードを返す
            return self.child_nodes[np.argmax(pucb_values)]

    # 現在の局面のノードの作成
    root_node = node(state, 0)

    # 複数回の評価を実行
    for _ in range(PV_EVALUATE_COUNT):
        root_node.evaluate()
    
    # 合法手の確率分布
    scores = nodes_to_scores(root_node.child_nodes)
    if temperature == 0: # 最大値のみ1
        action = np.argmax(scores)
        scores = np.zeros(len(scores))
        scores[action] = 1
    else: # ボルツマン分布でばらつき付加
        scores = boltzman(scores, temperature)
    return scores

# モンテカルロ木探索で行動選択
def pv_mcts_action(model, temperature=0):
    def pv_mcts_action(state:State):
        scores = pv_mcts_scores(model, state, temperature)
        return np.random.choice(state.get_list_of_legal_actions(), p=scores)
    return pv_mcts_action

# ボルツマン分布
def boltzman(xs, temperature):
    xs = [x ** (1 / temperature) for x in xs]
    return [x / sum(xs) for x in xs]

# 動作確認
if __name__ == '__main__':
    # モデルの読み込み
    path = sorted(Path('./model').glob('*.h5'))[-1]
    model = load_model(str(path))

    # 状態の生成
    state = State()

    # モンテカルロ木探索で行動取得を行う関数の生成
    next_action = pv_mcts_action(model, 1.0)

    # ゲーム終了までループ
    while True:
        # ゲーム終了時
        if state.is_done():
            break

        # 行動の取得
        action = next_action(state)

        # 次の状態の取得
        state = state.next(action)

        # 文字列表示
        board, area, enemy_area, walls = state.__str__()
        print("{}ターン目".format(state.get_game_count()))
        print("各職人の位置(A:味方の職人, E:敵の職人)")
        for i in range(HEIGHT):
            for j in range(WIDTH):
                print(board[i][j], end='')
            print()

        print("味方の領域を表示")
        for i in range(HEIGHT):
            for j in range(WIDTH):
                print(area[i][j], end='')
            print()
        print("敵の領域を表示")
        for i in range(HEIGHT):
            for j in range(WIDTH):
                print(enemy_area[i][j], end='')
            print()
        
        print("建てられた壁を表示(=:味方の壁, *:敵の壁)")
        for i in range(HEIGHT):
            for j in range(WIDTH):
                print(walls[i][j], end='')
            print()