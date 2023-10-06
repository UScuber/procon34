from game import State
from pv_mcts import pv_mcts_scores
from dual_network import DN_OUTPUT_SIZE
from datetime import datetime
from tensorflow.keras.models import load_model
from tensorflow.keras import backend as K
from pathlib import Path
import numpy as np
import pickle
import os
import gc

# パラメータの準備
SP_GAME_COUNT = 500 # セルフプレイを行うゲーム数(本家は25000)
SP_TEMPERATURE = 1.0 # ボルツマン分布の温度パラメータ

# 先手プレイヤーの価値
def first_player_value(ended_state):
    # 1:先手勝利、-1:先手敗北、0:引き分け
    if ended_state.is_lose():
        return -1 if ended_state.is_first_player() else 1
    return 0

# 学習データの保存
def write_data(history):
    now = datetime.now()
    os.makedirs('./data/', exist_ok=True) # フォルダがないときは生成
    path = './data/{:04}{:02}{:02}{:02}{:02}{:02}.history'.format(now.year, now.month, now.day, now.hour, now.minute, now.second)
    with open(path, mode='wb') as f:
        pickle.dump(history, f)

# 1ゲーム実行
def play(model):
    # 学習データ
    history = []

    # 状態の生成
    state = State()

    while True:
        # ゲーム終了時
        if state.is_done():
            break

        # 合法手の確率分布の取得
        scores = pv_mcts_scores(model, state, SP_TEMPERATURE)

        # 学習データに状態と方策を追加
        policies = [0] * DN_OUTPUT_SIZE
        for action, policy in zip(state.get_list_of_legal_actions(), scores):
            policies[action] = policy
        history.append([state.state_array(), policies, None])

        # 行動の取得
        action = np.random.choice(state.get_list_of_legal_actions(), p=scores)

        # 次の状態の取得
        state = state.next(action)

    # 学習データに勝ちを追加
    value = first_player_value(state)
    for i in range(len(history)):
        history[i][2] = value
        value = -value
    return history

# 複数回のゲームの実行
def running_game(model, history):
    for i in range(SP_GAME_COUNT):
        # 1ゲームの実行
        h = play(model)
        history.extend(h)

        # 出力
        print('\rSelfPlay {}/{}'.format(i+1, SP_GAME_COUNT), end='')

# セルフプレイ
def self_play():
    # 学習データ
    history = []

    # ベストプレイヤーのモデルの読み込み
    model = load_model('./model/best.h5')
    running_game(model, history)
    print('')

    # 学習データの保存
    write_data(history)

    # モデルの破棄
    del model
    K.clear_session()
    gc.collect()

if __name__ == '__main__':
    self_play()