## Introduction
This is an extension of the original sent2vec algorithm to produce word embeddings (see [Github page](https://github.com/epfml/sent2vec) and [paper](https://arxiv.org/abs/1703.02507)). It preserves all original functionality but makes it possible to listen to a Redis queue for input.

## Installation (Ubuntu 16)
```
sudo apt install build-essentials
https://github.com/salathegroup/sent2vec.git && cd sent2vec
git submodules
git submodule update --init --recursive
mkdir lib/cpp_redis/build && cd lib/cpp_redis/build
cmake .. -DCMAKE_BUILD_TYPE=Release
make 
make install
cd ../../
mkdir build
cmake ..
make
```

## Run code
Make sure Redis is running and set the following env vars (set the password to "" if not using a password):
```
export REDIS_HOST="***"
export REDIS_PORT="***"
export REDIS_PASSWORD="***"
```
Make sure to download a language model first (see [here](https://github.com/epfml/sent2vec#downloading-pre-trained-models))

Run the code using:
```
./sent2vec redis-mode <path to binary> <redis-input-queue-key>
```

Issue new work by opening `redis-cli` and type:
```
rpush <redis-input-queue-key> "\{\"text_tokenized\": \"this is my tokenized input string\", \"result_queue\": \"i3hzKK6dHG\"}"
```
The hash must have at least 2 arguments: 
* `text_tokenized`: Your tokenized text
* `result_queue`: Unique key name which will serve as the name of the output queue (you can use blpop to listen for this key)

Response:
```
> rpop i3hzKK6dHG
"{\"result_queue\":\"i3hzKK6dHG\",\"sentence_vector\":[0.259468257427216,-0.0617985092103481,..., -0.004367686342448,-0.0125288814306259],\"text_tokenized\":\"this is my tokenized input string\"}"
```
Use single_request mode to set a 10s expiry to key:
```
{
   "text_tokenized": "this is my tokenized input string",
   "result_queue": "i3hzKK6dHG",
   "mode": "single_request"
}
```
Obviously, using a client such as `redis-py` makes your life easier.

## Deploy
Simple deploy e.g. using PM2 and a launch script run.sh (containing `./sent2vec redis-mode <path to binary> <redis-input-queue-key>`)
```
npm install pm2
pm2 start run.sh
```

### Author
Martin Müller martin.muller@epfl.ch
