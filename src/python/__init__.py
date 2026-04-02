'''
# 1. Install dependencies
pip install -r requirements.txt

# 2. Start the C++ game (externalInputMode=true, stepMode=true in main.cpp)
./build/racer_game

# 3. Train (from src/python/)
cd src/python
python train.py

# 4. Monitor training
tensorboard --logdir runs/

# 5. Watch the trained agent
python play.py --model models/racer_ppo
'''