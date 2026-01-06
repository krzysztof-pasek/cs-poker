import tkinter as tk
import re
from network import Client
import time

class PokerGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Poker")
        self.root.geometry("700x600")
        self.root.configure(bg='white')
        
        self.hand_cards = []
        self.board_cards = []
        self.balance = 0
        self.pot = 0
        self.my_bet = 0
        self.highest_bet = 0
        self.timer_start_time = None
        self.timer_duration = 20
        self.timer_running = False
        self.folded = False
        
        self.client = None
        self.connected = False
        
        self.create_ui()
        self.auto_connect()
        self.process_messages()
        
    def create_ui(self):
        self.timer_label = tk.Label(self.root, text="Timer: --", bg='white', 
                                    fg='black', font=('Arial', 20, 'bold'))
        self.timer_label.pack(pady=10)
        
        info_frame = tk.Frame(self.root, bg='white')
        info_frame.pack()
        
        self.balance_label = tk.Label(info_frame, text="Saldo: 0", bg='white', 
                                     fg='black', font=('Arial', 12))
        self.balance_label.pack(side=tk.LEFT, padx=10)
        
        self.pot_label = tk.Label(info_frame, text="Pula: 0", bg='white', 
                                 fg='black', font=('Arial', 12))
        self.pot_label.pack(side=tk.LEFT, padx=10)
        
        self.my_bet_label = tk.Label(info_frame, text="Twoje: 0", bg='white', 
                                     fg='black', font=('Arial', 12))
        self.my_bet_label.pack(side=tk.LEFT, padx=10)
        
        tk.Label(self.root, text="Karty na stole:", bg='white', fg='black', 
                font=('Arial', 12, 'bold')).pack(pady=5)
        
        self.board_frame = tk.Frame(self.root, bg='white')
        self.board_frame.pack(pady=10)
        
        tk.Label(self.root, text="Twoje karty:", bg='white', fg='black', 
                font=('Arial', 12, 'bold')).pack(pady=5)
        
        self.hand_frame = tk.Frame(self.root, bg='white')
        self.hand_frame.pack(pady=10)
        
        self.status_label = tk.Label(self.root, text="", bg='white', fg='red', 
                                     font=('Arial', 10, 'bold'))
        self.status_label.pack(pady=5)
        
        self.winner_label = tk.Label(self.root, text="", bg='white', fg='green', 
                                     font=('Arial', 12, 'bold'))
        self.winner_label.pack(pady=5)
        
        action_frame = tk.Frame(self.root, bg='white', pady=20)
        action_frame.pack()
        
        self.bet_entry = tk.Entry(action_frame, width=10, font=('Arial', 12))
        self.bet_entry.pack(side=tk.LEFT, padx=5)
        
        self.bet_btn = tk.Button(action_frame, text="BET", command=self.place_bet, 
                                bg='lightgray', fg='black', state=tk.DISABLED, 
                                font=('Arial', 11))
        self.bet_btn.pack(side=tk.LEFT, padx=5)
        
        self.fold_btn = tk.Button(action_frame, text="FOLD", command=self.fold, 
                                 bg='lightgray', fg='black', state=tk.DISABLED,
                                 font=('Arial', 11))
        self.fold_btn.pack(side=tk.LEFT, padx=5)
        
    def auto_connect(self):
        self.client = Client("127.0.0.1", 8080)
        if self.client.connect():
            self.connected = True
            
    def place_bet(self):
        if not self.connected or self.folded:
            return
        try:
            amount = int(self.bet_entry.get())
            if amount > 0:
                self.client.send_message(f"BET {amount}")
        except:
            pass
            
    def fold(self):
        if self.connected and not self.folded:
            self.client.send_message("FOLD")
            self.folded = True
            self.bet_btn.config(state=tk.DISABLED)
            self.fold_btn.config(state=tk.DISABLED)
            self.status_label.config(text="SPASOWAŁEŚ - Obserwujesz grę")
            
    def parse_card(self, card_str):
        if not card_str or len(card_str) < 2:
            return None, None, None
        card_str = card_str.strip()
        
        if card_str.startswith('10'):
            rank = '10'
            suit = card_str[2] if len(card_str) > 2 else ''
        elif card_str[0].upper() == 'T':
            rank = '10'
            suit = card_str[1]
        else:
            rank = card_str[0]
            suit = card_str[1] if len(card_str) > 1 else ''
            
        suit_map = {
            'H': ('♥', 'red'),
            'D': ('♦', 'red'),
            'C': ('♣', 'black'),
            'S': ('♠', 'black'),
            'h': ('♥', 'red'),
            'd': ('♦', 'red'),
            'c': ('♣', 'black'),
            's': ('♠', 'black')
        }
        
        suit_info = suit_map.get(suit.upper(), (suit.upper(), 'black'))
        suit_symbol, suit_color = suit_info
        
        rank_map = {
            'J': 'J',
            'Q': 'Q',
            'K': 'K',
            'A': 'A'
        }
        display_rank = rank_map.get(rank.upper(), rank.upper())
        
        return display_rank, suit_symbol, suit_color
        
    def create_card_widget(self, parent, card_str):
        result = self.parse_card(card_str)
        if not result:
            return None
            
        rank, suit_symbol, suit_color = result
        
        card_frame = tk.Frame(parent, bg='white', relief=tk.RAISED, 
                              borderwidth=2, width=70, height=100)
        card_frame.pack(side=tk.LEFT, padx=5)
        card_frame.pack_propagate(False)
        
        rank_label = tk.Label(card_frame, text=rank, bg='white', fg=suit_color, 
                             font=('Arial', 14, 'bold'))
        rank_label.place(x=5, y=5)
        
        suit_label = tk.Label(card_frame, text=suit_symbol, bg='white', fg=suit_color, 
                             font=('Arial', 24, 'bold'))
        suit_label.place(relx=0.5, rely=0.5, anchor='center')
        
        return card_frame
        
    def update_cards_display(self):
        for widget in self.board_frame.winfo_children():
            widget.destroy()
        for widget in self.hand_frame.winfo_children():
            widget.destroy()
            
        if self.board_cards:
            for card in self.board_cards:
                self.create_card_widget(self.board_frame, card)
        else:
            tk.Label(self.board_frame, text="---", bg='white', fg='gray').pack()
            
        if self.hand_cards:
            for card in self.hand_cards:
                self.create_card_widget(self.hand_frame, card)
        else:
            tk.Label(self.hand_frame, text="---", bg='white', fg='gray').pack()
            
    def update_info_display(self):
        self.balance_label.config(text=f"Saldo: {self.balance}")
        self.pot_label.config(text=f"Pula: {self.pot}")
        self.my_bet_label.config(text=f"Twoje: {self.my_bet}")
        
    def update_timer(self):
        if not self.timer_running or not self.timer_start_time:
            return
            
        elapsed = time.time() - self.timer_start_time
        remaining = max(0, int(self.timer_duration - elapsed))
        
        if remaining > 0:
            self.timer_label.config(text=f"Timer: {remaining}s")
            self.root.after(100, self.update_timer)
        else:
            self.timer_running = False
            self.timer_label.config(text="Timer: 0s")
            if not self.folded:
                self.bet_btn.config(state=tk.DISABLED)
                self.fold_btn.config(state=tk.DISABLED)
            
    def start_timer(self, seconds=20):
        if not self.folded:
            self.timer_duration = seconds
            self.timer_start_time = time.time()
            self.timer_running = True
            self.bet_btn.config(state=tk.NORMAL)
            self.fold_btn.config(state=tk.NORMAL)
            self.update_timer()
        
    def stop_timer(self):
        self.timer_running = False
        self.timer_start_time = None
        self.timer_label.config(text="Timer: --")
        if not self.folded:
            self.bet_btn.config(state=tk.DISABLED)
            self.fold_btn.config(state=tk.DISABLED)
        
    def process_messages(self):
        if self.connected and self.client:
            while True:
                msg = self.client.get_message()
                if not msg:
                    break
                self.handle_message(msg)
        self.root.after(100, self.process_messages)
        
    def handle_message(self, msg):
        match = re.search(r'Your cards: (\w+)\s+(\w+)', msg)
        if match:
            self.hand_cards = [match.group(1), match.group(2)]
            self.update_cards_display()
            
        match = re.search(r'Card was revealed: (\w+)', msg)
        if match:
            card = match.group(1)
            if card not in self.board_cards:
                self.board_cards.append(card)
                self.update_cards_display()
                
        match = re.search(r'Balance: (\d+)', msg)
        if match:
            self.balance = int(match.group(1))
            self.update_info_display()
            
        match = re.search(r'Pot value: (\d+)', msg)
        if match:
            new_pot = int(match.group(1))
            pot_increase = new_pot - self.pot
            self.pot = new_pot
            
            if pot_increase > 0 and not self.folded:
                pass
                
            self.update_info_display()
            
        match = re.search(r'Bet accepted: (\d+)', msg)
        if match:
            bet_amount = int(match.group(1))
            self.my_bet += bet_amount
            if self.my_bet > self.highest_bet:
                self.highest_bet = self.my_bet
            self.update_info_display()
            
        match = re.search(r'Player: (\d+) placed (\d+) chips', msg)
        if match:
            bet_amount = int(match.group(2))
            new_highest = self.highest_bet + bet_amount
            if new_highest > self.highest_bet:
                self.highest_bet = new_highest
            self.update_info_display()
                
        if 'Paid entry fee' in msg:
            self.my_bet = 10
            self.highest_bet = 10
            if self.pot == 0:
                self.pot = 30
            self.update_info_display()
            
        if 'You folded' in msg:
            self.folded = True
            self.bet_btn.config(state=tk.DISABLED)
            self.fold_btn.config(state=tk.DISABLED)
            self.status_label.config(text="SPASOWAŁEŚ - Obserwujesz grę")
            
        if '20 seconds' in msg and not self.folded:
            self.start_timer(20)
            
        match = re.search(r'Player (\d+) wins pot: (\d+)', msg)
        if match:
            player_id = match.group(1)
            pot_amount = match.group(2)
            self.winner_label.config(text=f"Gracz {player_id} wygrywa pulę: {pot_amount} żetonów!")
            
        match = re.search(r'Winner by showdown: Player (\d+)', msg)
        if match:
            player_id = match.group(1)
            current_text = self.winner_label.cget("text")
            if current_text:
                self.winner_label.config(text=current_text + f" | Gracz {player_id}")
            else:
                self.winner_label.config(text=f"Zwycięzca: Gracz {player_id}")
                
        match = re.search(r'Player (\d+) wins (\d+) chips!', msg)
        if match:
            player_id = match.group(1)
            chips = match.group(2)
            current_text = self.winner_label.cget("text")
            if current_text:
                self.winner_label.config(text=current_text + f" | Wygrywa {chips} żetonów")
            else:
                self.winner_label.config(text=f"Gracz {player_id} wygrywa {chips} żetonów!")
            
        if 'New round starting' in msg:
            self.board_cards = []
            self.hand_cards = []
            self.my_bet = 0
            self.highest_bet = 0
            self.folded = False
            self.status_label.config(text="")
            self.winner_label.config(text="")
            self.update_cards_display()
            self.update_info_display()
            
        if 'wins' in msg.lower() or 'Game Over' in msg:
            self.stop_timer()

def main():
    root = tk.Tk()
    app = PokerGUI(root)
    root.mainloop()

if __name__ == "__main__":
    main()
