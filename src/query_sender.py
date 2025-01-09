from youtube_search import YoutubeSearch
import argparse
import json

parser = argparse.ArgumentParser()

parser.add_argument("-q", "--query", type=str, required=True)
parser.add_argument("-l", "--limit", type=int, default=5)
parser.add_argument("-c", "--config", type=str)

args = parser.parse_args()


results = YoutubeSearch(args.query, max_results=args.limit).to_json()

try:
    json_data = json.loads(results)
    
    file_name = f"{args.config}/tmp/\'{args.query}\'.json"

    with open(file_name, "w", encoding="utf-8") as file:
        json.dump(json_data, file, ensure_ascii=False, indent=4)

except json.JSONDecodeError as e:
    print("Error")

