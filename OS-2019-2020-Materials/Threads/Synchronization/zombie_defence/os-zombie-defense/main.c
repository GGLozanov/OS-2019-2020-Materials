#include "ui.h"
#include <pthread.h>
#include <unistd.h>
#define GOLD_MARGIN 10
#define false 0
#define true 1

pthread_mutex_t gold_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t soldier_mutex = PTHREAD_MUTEX_INITIALIZER;
static long gold_storage = 100;
static int soldier_count = 0;
static int zombie_count = 1;
static int player_health = 100;

void* mine(void* args) {
	while(true) {
		pthread_mutex_lock(&gold_mutex);
		gold_storage += GOLD_MARGIN;
		print_gold(gold_storage);
		pthread_mutex_unlock(&gold_mutex);
		sleep(1);
	}
}

void miner_create(pthread_t* miner) {
	pthread_mutex_lock(&gold_mutex);
	if(gold_storage >= 100) {
		gold_storage -= 100;
		print_gold(gold_storage);
		print_msg("Miner created!");
		pthread_create(miner, NULL, mine, NULL);
	} else print_fail("Not enough gold!");
	pthread_mutex_unlock(&gold_mutex);

}

void be_single_soldier() {
	pthread_mutex_lock(&gold_mutex);
	if(gold_storage >= 10) {
		gold_storage -= 10;
		print_gold(gold_storage);
		print_msg("Soldier created!");
		print_soldiers(++soldier_count);
	} else print_fail("Not enough gold!");
	pthread_mutex_unlock(&gold_mutex);
}

void be_ten_soldiers() {
	pthread_mutex_lock(&gold_mutex);
	if(gold_storage >= 100) {
		gold_storage -= 100;
		print_gold(gold_storage);
		print_msg("10 x soldiers created!");
		soldier_count += GOLD_MARGIN;
		print_soldiers(soldier_count);
	} else print_fail("Not enough gold!");
	pthread_mutex_unlock(&gold_mutex);
}

void *be_zombie(void* args) {
	while(true) {
		for(int i = 5; i > 0; i--) {
			pthread_mutex_lock(&gold_mutex);
			print_zombies(i, zombie_count);
			pthread_mutex_unlock(&gold_mutex);
			sleep(1);
		}
		
		pthread_mutex_lock(&soldier_mutex);
		if(zombie_count > soldier_count) {
			player_health -= zombie_count - soldier_count;
			print_health(player_health);		
			print_fail("Zombie attack succeded ;(!");
			if(player_health <= 0) {
				game_end(zombie_count);
			}
		} else {
			print_succ("Zombie attack deflected! :)");
		}

		zombie_count *= 2;
		print_zombies(5, zombie_count);
		pthread_mutex_unlock(&soldier_mutex);
	}
}

int main() {
	init();
	print_zombies(5, zombie_count);
	print_soldiers(soldier_count);
	print_health(player_health);
	print_gold(gold_storage);
	
	pthread_t miners, soldiers, zombies;
	
	pthread_create(&zombies, NULL, be_zombie, NULL);

	while(true) {
		int ch = get_input();
		switch(ch) {
			case 'q':
				game_end(1);
				break;
			case 'm':
				// create... thread...
				miner_create(&miners);
				break;
			case 's':
				be_single_soldier();
				break;
			case 'x':
				be_ten_soldiers();
				break;
		}
	}
	
	pthread_join(miners, NULL);
	pthread_join(soldiers, NULL);
	pthread_join(zombies, NULL);	
}
