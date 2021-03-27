#include <neuz/neuz.h>

#define N0 2
#define N1 5
#define N2 4

#define NT  10000
#define RATE    0.1

double loss(zVec output, zVec des)
{
  return 0.5 * zVecSqrDist( output, des );
}

double lossgrad(zVec output, zVec des, int i)
{
  return zVecElem(output,i) - zVecElem(des,i);
}

double train(nzNet *net, zVec input, zVec output, zVec des, int i1, int i2, int oo, int oa, int on, int ox)
{
  zVecSetElemList( input, (double)i1, (double)i2 );
  zVecSetElemList( des, (double)oo, (double)oa, (double)on, (double)ox );
  nzNetPropagate( net, input );
  nzNetGetOutput( net, output );
  nzNetBackPropagate( net, input, des, lossgrad );
  return loss( output, des );
}

void test(nzNet *net, zVec input, zVec output, int i1, int i2)
{
  zVecSetElemList( input, (double)i1, (double)i2 );
  nzNetPropagate( net, input );
  nzNetGetOutput( net, output );
  printf( "I1=%g, I2=%g -> OR: %g, AND: %g, NAND: %g, XOR: %g\n", zVecElemNC(input,0), zVecElemNC(input,1), zVecElemNC(output,0), zVecElemNC(output,1), zVecElemNC(output,2), zVecElemNC(output,3) );
}

int main(int argc, char *argv[])
{
  nzNet nn;
  zVec input, output, des;
  double l;
  int i;

  zRandInit();

  input  = zVecAlloc( N0 );
  output = zVecAlloc( N2 );
  des = zVecAlloc( N2 );

  nzNetInit( &nn );
  nzNetAddGroupSetActivator( &nn, N0, NULL );
#if 1
  nzNetAddGroupSetActivator( &nn, N1, &nz_activator_sigmoid );
  nzNetAddGroupSetActivator( &nn, N2, &nz_activator_sigmoid );
#else
  nzNetAddGroupSetActivator( &nn, N1, &nz_activator_relu );
  nzNetAddGroupSetActivator( &nn, N2, &nz_activator_relu );
#endif
  nzNetConnectGroup( &nn, 0, 1 );
  nzNetConnectGroup( &nn, 1, 2 );

  /* training */
  for( i=0; i<NT; i++ ){
    nzNetInitGrad( &nn );
    l  = train( &nn, input, output, des, 0, 0, 0, 0, 1, 0 );
    l += train( &nn, input, output, des, 1, 0, 1, 0, 1, 1 );
    l += train( &nn, input, output, des, 0, 1, 1, 0, 1, 1 );
    l += train( &nn, input, output, des, 1, 1, 1, 1, 0, 0 );
    printf( "%03d %.10g\n", i, l );
    if( zIsTiny( l ) ) break;
    nzNetTrainSDM( &nn, RATE );
  }

  test( &nn, input, output, 0, 0 );
  test( &nn, input, output, 1, 0 );
  test( &nn, input, output, 0, 1 );
  test( &nn, input, output, 1, 1 );

  nzNetDestroy( &nn );
  zVecFreeAO( 3, input, output, des );
  return 0;
}
