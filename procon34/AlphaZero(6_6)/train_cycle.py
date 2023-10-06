from dual_network import dual_network
from self_play import self_play
from train_network import train_network
from evaluate_network import evaluate_network
import os
import tensorflow as tf

# デュアルネットワークの作成
dual_network()

for i in range(10):
    print('Train',i,'===================')
    # セルフプレイ部
    self_play()

    # パラメータ更新部
    train_network()

    # ネットワークの評価
    update_best_player = evaluate_network()