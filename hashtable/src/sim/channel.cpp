#include <unistd.h>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread/thread.hpp>

using namespace std;

template <typename T, int N>
class channel {
private:
  boost::lockfree::queue<T, boost::lockfree::capacity<N> > Q;
public:
  void block_push(T const &val) {
    while (!Q.push(val)) {}
  }
  bool push(T const &val) {
    return Q.push(val);
  }
  T block_pop() {
    T ret;
    while (!Q.pop(ret)) {}
    return ret;
  }
  T pop(bool *success) {
    T ret;
    *success = Q.pop(ret);
    return ret;
  }
  bool empty() {
    return Q.empty();
  }
};

template <typename T, int N>
T read_channel_nb_altera(channel<T, N> &Q, bool *success) {
  T ret = Q.pop(success);
  return ret;
}

template <typename T, int N>
T read_channel_altera(channel<T, N> &Q) {
  T ret = Q.block_pop();
  return ret;
}

template <typename T, int N>
bool write_channel_nb_altera(channel<T, N> &Q, T const &val) {
  bool ret = Q.push(val);
  return ret;
}

template <typename T, int N>
void write_channel_altera(channel<T, N> &Q, T const &val) {
  Q.block_push(val);
}
