/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <iostream>

#include "fasttext.h"
#include <cpp_redis/core/client.hpp>
#include <cpp_redis/core/subscriber.hpp>
#include <cpp_redis/misc/error.hpp>
#include <array>
#include <sstream>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#include "../lib/json.hpp"

using namespace fasttext;

void printUsage() {
  std::cerr
    << "usage: fasttext <command> <args>\n\n"
    << "The commands supported by fasttext are:\n\n"
    << "  supervised              train a supervised classifier\n"
    << "  sent2vec                train unsupervised sentence embeddings\n"
    << "  quantize                quantize a model to reduce the memory usage\n"
    << "  test                    evaluate a supervised classifier\n"
    << "  predict                 predict most likely labels\n"
    << "  predict-prob            predict most likely labels with probabilities\n"
    << "  skipgram                train a skipgram model\n"
    << "  cbow                    train a cbow model\n"
    << "  print-word-vectors      print word vectors given a trained model\n"
    << "  print-sentence-vectors  print sentence vectors given a trained model\n"
    << "  nn                      query for nearest neighbors\n"
    << "  nnSent                  query for nearest neighbors for sentences\n"
    << "  analogies               query for analogies\n"
    << "  analogiesSent           query for analogies for Sentences\n"
    << std::endl;  
}

void printQuantizeUsage() {
  std::cerr
    << "usage: fasttext quantize <args>"
    << std::endl;
}

void printTestUsage() {
  std::cerr
    << "usage: fasttext test <model> <test-data> [<k>]\n\n"
    << "  <model>      model filename\n"
    << "  <test-data>  test data filename (if -, read from stdin)\n"
    << "  <k>          (optional; 1 by default) predict top k labels\n"
    << std::endl;
}

void printPredictUsage() {
  std::cerr
    << "usage: fasttext predict[-prob] <model> <test-data> [<k>]\n\n"
    << "  <model>      model filename\n"
    << "  <test-data>  test data filename (if -, read from stdin)\n"
    << "  <k>          (optional; 1 by default) predict top k labels\n"
    << std::endl;
}

void printPrintWordVectorsUsage() {
  std::cerr
    << "usage: fasttext print-word-vectors <model>\n\n"
    << "  <model>      model filename\n"
    << std::endl;
}

void printPrintSentenceVectorsUsage() {
  std::cerr
    << "usage: fasttext print-sentence-vectors <model>\n\n"
    << "  <model>      model filename\n"
    << std::endl;
}

void printRedisModeVectorsUsage() {
  std::cerr
          << "usage: fasttext print-sentence-vectors sent2vec redis-mode <model> <input_list> <output_list>\n\n"
          << "  <model>        model filename\n"
          << "  <input_list>   key in redis with list of inputs\n"
          << "  <output_list>   key in redis with list of outputs\n"
          << std::endl;
}


void printPrintNgramsUsage() {
  std::cerr
    << "usage: fasttext print-ngrams <model> <word>\n\n"
    << "  <model>      model filename\n"
    << "  <word>       word to print\n"
    << std::endl;
}

void quantize(int argc, char** argv) {
  std::shared_ptr<Args> a = std::make_shared<Args>();
  if (argc < 3) {
    printQuantizeUsage();
    a->printHelp();
    exit(EXIT_FAILURE);
  }
  a->parseArgs(argc, argv);
  FastText fasttext;
  fasttext.quantize(a);
  exit(0);
}

void printNNUsage() {
  std::cout
    << "usage: fasttext nn <model> <k>\n\n"
    << "  <model>      model filename\n"
    << "  <k>          (optional; 10 by default) predict top k labels\n"
    << std::endl;
}

void printNNSentUsage() {
  std::cerr
    << "usage: fasttext nnSent <model> <corpus> <k>\n\n"
    << "  <model>      model filename\n"
    << "  <corpus>     corpus filename \n"
    << "  <k>          (optional; 10 by default) predict top k labels\n"
    << std::endl;
    std::cout<<"NOTE : A corpus file is required to find similar sentences."<<std::endl;
}

void printAnalogiesUsage() {
  std::cout
    << "usage: fasttext analogies <model> <k>\n\n"
    << "  <model>      model filename\n"
    << "  <k>          (optional; 10 by default) predict top k labels\n"
    << std::endl;
}

void printAnalogiesSentUsage() {
  std::cout
    << "usage: fasttext analogiesSent <model> <corpus> <k>\n\n"
    << "  <model>      model filename\n"
    << "  <corpus>     corpus filename \n"
    << "  <k>          (optional; 10 by default) predict top k labels\n"
    << std::endl;
  std::cout<<"NOTE : A corpus file is required to find similar sentences."<<std::endl;
}

void test(int argc, char** argv) {
  if (argc < 4 || argc > 5) {
    printTestUsage();
    exit(EXIT_FAILURE);
  }
  int32_t k = 1;
  if (argc >= 5) {
    k = atoi(argv[4]);
  }

  FastText fasttext;
  fasttext.loadModel(std::string(argv[2]));

  std::string infile(argv[3]);
  if (infile == "-") {
    fasttext.test(std::cin, k);
  } else {
    std::ifstream ifs(infile);
    if (!ifs.is_open()) {
      std::cerr << "Test file cannot be opened!" << std::endl;
      exit(EXIT_FAILURE);
    }
    fasttext.test(ifs, k);
    ifs.close();
  }
  exit(0);
}

void predict(int argc, char** argv) {
  if (argc < 4 || argc > 5) {
    printPredictUsage();
    exit(EXIT_FAILURE);
  }
  int32_t k = 1;
  if (argc >= 5) {
    k = atoi(argv[4]);
  }

  bool print_prob = std::string(argv[1]) == "predict-prob";
  FastText fasttext;
  fasttext.loadModel(std::string(argv[2]));

  std::string infile(argv[3]);
  if (infile == "-") {
    fasttext.predict(std::cin, k, print_prob);
  } else {
    std::ifstream ifs(infile);
    if (!ifs.is_open()) {
      std::cerr << "Input file cannot be opened!" << std::endl;
      exit(EXIT_FAILURE);
    }
    fasttext.predict(ifs, k, print_prob);
    ifs.close();
  }

  exit(0);
}

void printWordVectors(int argc, char** argv) {
  if (argc != 3) {
    printPrintWordVectorsUsage();
    exit(EXIT_FAILURE);
  }
  FastText fasttext;
  fasttext.loadModel(std::string(argv[2]));
  fasttext.printWordVectors();
  exit(0);
}

void printSentenceVectors(int argc, char** argv) {
  if (argc != 3) {
    printPrintSentenceVectorsUsage();
    exit(EXIT_FAILURE);
  }
  FastText fasttext;
  fasttext.loadModel(std::string(argv[2]));
  fasttext.printSentenceVectors();
  exit(0);
}

void printNgrams(int argc, char** argv) {
  if (argc != 4) {
    printPrintNgramsUsage();
    exit(EXIT_FAILURE);
  }
  FastText fasttext;
  fasttext.loadModel(std::string(argv[2]));
  fasttext.ngramVectors(std::string(argv[3]));
  exit(0);
}

void nn(int argc, char** argv) {
  int32_t k;
  if (argc == 3) {
    k = 10;
  } else if (argc == 4) {
    k = atoi(argv[3]);
  } else {
    printNNUsage();
    exit(EXIT_FAILURE);
  }
  FastText fasttext;
  fasttext.loadModel(std::string(argv[2]));
  fasttext.nn(k);
  exit(0);
}

void nnSent(int argc, char** argv) {
  int32_t k;
  if (argc == 4) {
    k = 10;
  } else if (argc == 5) {
    k = atoi(argv[4]);
  } else {
    printNNSentUsage();
    exit(EXIT_FAILURE);
  }
  FastText fasttext;
  fasttext.loadModel(std::string(argv[2]));
  fasttext.nnSent(k,std::string(argv[3]));
  exit(0);
}


void analogies(int argc, char** argv) {
  int32_t k;
  if (argc == 3) {
    k = 10;
  } else if (argc == 4) {
    k = atoi(argv[3]);
  } else {
    printAnalogiesUsage();
    exit(EXIT_FAILURE);
  }
  FastText fasttext;
  fasttext.loadModel(std::string(argv[2]));
  fasttext.analogies(k);
  exit(0);
}

void analogiesSent(int argc, char** argv) {
  int32_t k;
  if (argc == 4) {
    k = 10;
  } else if (argc == 5) {
    k = atoi(argv[4]);
  } else {
    printAnalogiesSentUsage();
    exit(EXIT_FAILURE);
  }
  FastText fasttext;
  fasttext.loadModel(std::string(argv[2]));
  fasttext.analogiesSent(k,std::string(argv[3]));
  exit(0);
}

void train(int argc, char** argv) {
  std::shared_ptr<Args> a = std::make_shared<Args>();
  a->parseArgs(argc, argv);
  FastText fasttext;
  fasttext.train(a);
}

void redisMode(int argc, char** argv) {
  if (argc != 5) {
    printRedisModeVectorsUsage();
    exit(EXIT_FAILURE);
  }

  cpp_redis::client client;
  client.connect("127.0.0.1", 6379);
  cpp_redis::active_logger = std::unique_ptr<cpp_redis::logger>(new cpp_redis::logger);

  if (client.is_connected()) {
    cpp_redis::active_logger->info("Successfully connected to Redis.", __FILENAME__, __LINE__);
  }

  // Queue Names
  const std::vector<std::string> redis_listen_queue{std::string(argv[3])};
  const std::string redis_result_queue{std::string(argv[4])};

  FastText fasttext;
  cpp_redis::active_logger->info("Loading model...", __FILENAME__, __LINE__);
  fasttext.loadModel(std::string(argv[2]));
  cpp_redis::active_logger->info("... done", __FILENAME__, __LINE__);

  while(client.is_connected()) {
    // Get new text_obj
    auto response = client.blpop(redis_listen_queue,3600);
    client.sync_commit();
    response.wait();
    auto reply = response.get();
    auto reply_arr = reply.as_array();
    std::string resp_str = reply_arr[1].as_string();

    // parse into JSON
    nlohmann::json text_obj;
    try {
      text_obj = nlohmann::json::parse(resp_str);
    } catch(nlohmann::detail::type_error) {
      cpp_redis::active_logger->error("Could not parse string to JSON", __FILENAME__, __LINE__);
      continue;
    }
    long obj_id = text_obj["id"];
    std::string obj_id_str = std::to_string(obj_id);
    std::string msg = "Received text_obj with id " + obj_id_str;
    cpp_redis::active_logger->info(msg, __FILENAME__, __LINE__);

    // Compute sentence vector
    std::cout << text_obj << std::endl;
    if (text_obj["text_tokenized"] == NULL || text_obj["text_tokenized"] == "") {
      cpp_redis::active_logger->error("text_tokenized field is empty", __FILENAME__, __LINE__);
      continue;
    }
    std::string text = text_obj["text_tokenized"];
    Vector result = fasttext.singleSentenceVector(text);
    std::vector<float> embedding_vector = {};

    // Read out result into vector
    for (int64_t j = 0; j < result.m_; j++) {
      if (std::isnan(result.data_[j])) {
        embedding_vector = {};
        break;
      } else {
        embedding_vector.push_back(static_cast<float>(result.data_[j]));
      }
    }
    text_obj["sentence_vector"] = embedding_vector;
    std::cout << text_obj << std::endl;

    // Push to result queue
    std::vector<std::string> text_obj_dump = {};
    try {
      text_obj_dump = {text_obj.dump()};
    } catch(nlohmann::detail::type_error) {
      cpp_redis::active_logger->error("Could not convert JSON to string", __FILENAME__, __LINE__);
      continue;
    }

    // Push to result queue
    client.rpush(redis_result_queue, text_obj_dump);
  }
}


int main(int argc, char** argv) {
  if (argc < 2) {
    printUsage();
    exit(EXIT_FAILURE);
  }
  std::string command(argv[1]);
  if (command == "skipgram" || command == "cbow" || command == "supervised" ||
      command == "sent2vec") {
    train(argc, argv);
  } else if (command == "test") {
    test(argc, argv);
  } else if (command == "quantize") {
    quantize(argc, argv);
  } else if (command == "print-word-vectors") {
    printWordVectors(argc, argv);
  } else if (command == "print-sentence-vectors") {
    printSentenceVectors(argc, argv);
  } else if (command == "print-ngrams") {
    printNgrams(argc, argv);
  } else if (command == "nn") {
    nn(argc, argv);
  } else if (command == "nnSent") {
    nnSent(argc, argv);
  } else if (command == "analogies") {
    analogies(argc, argv);
  } else if (command == "analogiesSent") {
    analogiesSent(argc, argv);
  } else if (command == "predict" || command == "predict-prob" ) {
    predict(argc, argv);
  } else if (command == "redis-mode") {
    redisMode(argc, argv);
  } else {
    printUsage();
    exit(EXIT_FAILURE);
  }
  return 0;
}
