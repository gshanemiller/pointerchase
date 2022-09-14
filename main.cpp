#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>

#include <string>
#include <vector>

#include <intel_pmu_stats.h>
#include <intel_skylake_pmu.h>

const int NodeWidth = 122-65+1;
Intel::SkyLake::PMU pmu(false, Intel::SkyLake::PMU::ProgCounterSetConfig::k_DEFAULT_SKYLAKE_CONFIG_0);

template <int T>
struct Node {
  // DATA
  Node *d_children[T];
  u_int64_t nodeCount;

  // CREATOR
  Node() {
    nodeCount = 0;
    memset(d_children, 0, sizeof(d_children));
  }

  // ACCESSORS
  int find(const char *str, u_int32_t size) {
    assert(str);
    assert(size>0);

    Node *lastRoot = this;

    for (u_int32_t i=0; i<size; ++i) {
      const char ch = str[i]-'A';
      assert(ch>=0&&ch<T);
      if (lastRoot->d_children[ch] == 0) {
        return -1;
      }
      lastRoot = lastRoot->d_children[ch];
    }
    return 0;
  }

  // MANIPULATORS
  void insert(const char *str, u_int32_t size) {
    assert(str);
    assert(size>0);

    Node *lastRoot = this;

    for (u_int32_t i=0; i<size; ++i) {
      const char ch = str[i]-'A';
      assert(ch>=0&&ch<T);
      if (lastRoot->d_children[ch] == 0) {
        lastRoot->d_children[ch] = new Node();
        ++nodeCount;
      }
      lastRoot = lastRoot->d_children[ch];
    }
  }
};

int main(int argc, char **argv) {
  if (argc!=3) {
    printf("usage: t <count> <file>\n");
    exit(2);
  }

  printf("sizeof(Node<%d>) == %lu\n", NodeWidth, sizeof(Node<NodeWidth>));

  std::vector<std::string*> str(atoi(argv[1]));
  printf("str capacity: %lu size %lu\n", str.capacity(), str.size());

  u_int32_t i=0;
  char buffer[128];
  FILE *fid = fopen(argv[2], "r");

  if (fid==0) {
    printf("bad file\n");
    exit(2);
  }
  printf("reading %s...\n", argv[2]);
  while (!feof(fid)) {
    if (fgets(buffer, sizeof(buffer)-1, fid)!=0) {
      u_int32_t sz = strlen(buffer)-1;
      str[i] = new std::string(buffer, sz); 
      // printf("%u read (%s)...\n", i, str[i]->c_str());
      if ((++i%100000)==0) {
        printf("read %u...\n", i);
      }
    }
  }
  fclose(fid);

  timespec start, end;                                                                                                  
  Intel::Stats findStats;
  Intel::Stats insertStats;
  Node<NodeWidth> *root = new Node<NodeWidth>;

  pmu.reset();
  pmu.start();                                                                                                       
  timespec_get(&start, TIME_UTC); 
  for (u_int32_t k=0; k<str.size(); ++k) {
    root->insert(str[k]->c_str(), str[k]->length());
  }
  timespec_get(&end, TIME_UTC);                                                                                       
  insertStats.record("Insert", str.size(), start, end, pmu);
  insertStats.dumpScaled(pmu);

  printf("%lu nodes created\n", root->nodeCount);

  pmu.reset();
  pmu.start();                                                                                                       
  timespec_get(&start, TIME_UTC); 
  for (u_int32_t k=0; k<str.size(); ++k) {
    if (root->find(str[k]->c_str(), str[k]->length())!=0) {
      printf("error\n");
    }
  }
  timespec_get(&end, TIME_UTC);                                                                                       
  findStats.record("Find", str.size(), start, end, pmu);
  findStats.dumpScaled(pmu);
  
  return 0;
}
