import os

mere_check = False

round_list = open('day_table.txt', 'r', encoding='utf-8').read().splitlines()
round_list = [r.split("\t") for r in round_list]

if mere_check:
	missing = ""

	for rnd in round_list:
		season_name = " ".join(rnd[0].split(" ")[:-1]).strip().replace("&", "_").replace("'", "_")
		season_name = season_name.encode(encoding="utf-8").decode(encoding="ansi", errors="ignore")
		round_number = rnd[0].split(" R")[-1]

		if season_name not in os.listdir("toconvertdata/"):
			missing += rnd[0] + "\n"
			continue

		if round_number + ".txt" not in os.listdir(f'toconvertdata/{season_name}'):
			missing += rnd[0] + "\n"
			continue
	
	print("Missing round count:", missing.count("\n"))
	open("missing_rounds.txt", "w", encoding="utf-8").write(missing)
	quit()

i = 0
for rnd in round_list:
	season_name = " ".join(rnd[0].split(" ")[:-1]).strip().replace("&", "_").replace("'", "_")
	season_name = season_name.encode(encoding="utf-8").decode(encoding="ansi", errors="ignore")
	round_number = rnd[0].split(" R")[-1]

	if season_name not in os.listdir("toconvertdata/"):
		continue

	if round_number + ".txt" not in os.listdir(f"toconvertdata/{season_name}"):
		continue
	
	if season_name not in os.listdir("dailydata/"):
		os.makedirs(f"dailydata/{season_name}")

	round_data = open(f"toconvertdata/{season_name}/{round_number}.txt", "r", encoding="utf-8").read().splitlines()[1:]
	round_data = [rnd[1]] + round_data
	round_data = "\n".join(round_data)

	open(f"dailydata/{season_name}/{round_number}.txt", "w", encoding="utf-8").write(round_data)

	i += 1
	print(f"{i} rounds added...", end="\r")