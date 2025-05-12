import json
import urllib.request

from symbolchain.facade.SymbolFacade import SymbolFacade
from symbolchain.symbol.Network import NetworkTimestamp

facade = SymbolFacade('mainnet')
print(f"Network name: {facade.network.name}")
# NetworkTimestamp(0) is the genesis block timestamp
launch_date = facade.network.to_datetime(NetworkTimestamp(0))
print(f"Network launch date: {launch_date}")

NODE_URL = 'https://001-sai-dual.symboltest.net:3001'
print(f'Using node {NODE_URL}')
try:
    # Fetch current chain information
    info_path = '/chain/info'
    print(f'Fetching chain information from {info_path}')
    with urllib.request.urlopen(
        f'{NODE_URL}{info_path}', timeout = 10
    ) as response:
        response_json = json.loads(response.read().decode())
        height = int(response_json['height'])
        print(f"  Blockchain height: {height:,} blocks")

except urllib.error.URLError as e:
    print(e.reason)
