#include "evar.c"
#include "rnd.c"

//   states   |        raw data         +    weight pointers
//  v*    v   |  m  n  b         w      +   w**      w*     b*
// ---- ----- | --- - ----- ----------- + ------- -------- -----
// nF*  nΣmF  | nu  u v-m1  n-1Σmimi+1F + n-1F**  n-1ΣmF*  n-1F*

typedef struct NeuralNetwork {
	u64 bsize; //byte size (only raw data that pointers can be made for)
	u32 size, *m; //layer num & sizes
	F32 **val; //activation of each neuron (stored independently)
	F32 ***w, **b; //weights & biases
} nnet;

//returns a Neural network struct using the format above
//n = layer amount, m = size of each one, v = whether to allocate states for the neurons
nnet NeuralNet(u32 n, u32 *m, const u32 v) {
	//get size of components that need sums
	u64 msum=m[n-1]; //Sum of neurons
	u64 wsum=0; //Sum of weights
	for (u32 i=0; i<n-1; i++) {
		msum += m[i]; //[n-1]Σmi
		wsum += m[i]*m[i+1]; //[n-1]Σmimi+1
	}
	u64 rawsize = (n+1+msum-m[0]+wsum)*sizeof(u32); //size of n+m+b+w (assuming F32 == u32)
	nnet net = (nnet){rawsize, n, malloc(rawsize+(2*(n-1)+msum-m[n-1])*sizeof(F32*))};
	if (v) { //if statement is useful for when having many copies of the same network
		net.val = malloc(n*sizeof(F32*)+msum*sizeof(F32));
		net.val[0] = (F32*)(net.val+n);
		for (u32 i=0; i<n-1; i++)
			net.val[i+1] = net.val[i]+m[i];
	}
	for (u32 i=0; i<n; i++)
		net.m[i] = m[i]; //copy over all sizes of each layer
	net.m[n] = n; //copy over size to serialized section
	F32* b = (F32*)(net.m+n+1);
	for (u32 i=0; i<msum-m[0]; i++)
		b[i] = rnd(-1,1); //generate biases
	n--; //everything bellow uses n-1
	F32 *w = b + msum-m[0]; //refer to sheet above Neural Net struct (w)
	net.w = (F32***)(w + wsum); // (w**)
	F32 **wP = (F32**)net.w + n; // (w*)
	net.b = wP+msum-m[n];
	for (u32 i=0; i<n; i++) {
		net.b[i] = b;
		b += m[i+1];
		net.w[i] = wP;
		for (u32 j=0; j<m[i]; j++) {
			wP[j] = w;
			for (u32 k=0; k<m[i+1]; k++)
				w[k] = rnd(-1,1);
			w += m[i+1];
		}
		wP += m[i];
	}
	return net;
}

void nn_run(nnet *net) { //process the neural network's input
	F32 *next = net->val[0], *prev; //pointers to next & previous layer values
	for (u32 i=0; i<net->size-1; i++) {
		prev = next; //next layer becomes previous one
		next = net->val[i+1];
		for (u32 j=0; j<net->m[i+1]; j++) {
			next[j] = net->b[i][j]; //start with bias
			for (u32 k=0; k<net->m[i]; k++)
				next[j] += prev[k] * net->w[i][j][k]; //add weights of all previous neurons
			next[j] /= next[j]*next[j]*0.25+1; //apply an activation function
		}
	}
}

//copy params from A to B
void nn_copy(nnet *A, nnet *B) {
	u64 len = (A->bsize - (A->size+1)*sizeof(u32)) / sizeof(F32); //num of params
	F32 *pA = A->b[0];
	F32 *pB = B->b[0];
	for (u32 i=0; i<len; i++)
		pB[i] = pA[i];
}

//copy params from A to B with mutation
//min_chance is the minimum random number from 0-1 needed to trigger a mutation
//amplitude is how much they will mutate (0.5 means they will change from -0.5 to 0.5)
void nn_copy_mutate(nnet *A, nnet *B, F32 min_chance, F32 amplitude) {
	u64 len = (A->bsize - (A->size+1)*sizeof(u32)) / sizeof(F32); //num of params
	F32 *pA = A->b[0];
	F32 *pB = B->b[0];
	for (u32 i=0; i<len; i++)
		if (rand() * (1./RAND_MAX) >= min_chance)
			pB[i] = pA[i] + (rand()*(2./RAND_MAX) - 1.) * amplitude;
}

#define ainum 16
#define trainrounds 5000

int main() {
	struct timeval stime;
	gettimeofday(&stime, NULL);
	srand(stime.tv_usec);

	u32 m[3] = {2,3,1};
	nnet n[ainum] = {NeuralNet(3,m,1)};
	F32 best;
	u32 b_at = 0;
	for (u32 i=1; i<ainum; i++) {
		n[i] = NeuralNet(3,m,0);
		n[i].val = n[0].val;
	}

	for (u32 i=0; i<100000; i++) //test training
		for (u32 j=0; j<2; j++)
			for (u32 k=0; k<2; k++) {
				best = 4;
				n[0].val[0][0] = j;
				n[0].val[0][1] = k;
				for (u32 w=0; w<ainum; w++) {
					nn_run(&n[w]);
					const F32 tmp = fabsf((j^k) - n[w].val[2][0]);
					if (tmp < best) {
						best = tmp;
						b_at = w;
					}
				}
				if (i%100==0) {
					printf("in: %u %u, out[0]: %f\n",j,k,n[0].val[2][0]);
					printf("%f\n",best);
				}
				for (u32 w=(b_at+1)%ainum; w!=b_at; w=(w+1)%ainum)
					nn_copy_mutate(&n[b_at], &n[w], 0.3, 0.08-(i*0.079/100000.));
			}
}
