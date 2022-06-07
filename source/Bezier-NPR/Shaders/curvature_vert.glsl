#version 410 core

layout (location = 0) in vec3 position;

out vec4 geometry_position;

out float curv_value;
out vec4 geometry_min_dir;
out vec4 geometry_max_dir;
out vec4 geometry_normale;
out vec3 geometry_k1_k2;

float u_lvl = 1.0f;
float u_curv_radius = 2.0f;
uniform sampler3D u_xyz2_tex;
uniform sampler3D u_xy_yz_xz_tex;
uniform sampler3D u_xyz_tex;

int getVolumeMoments(in vec3 vertex_position, out float volume, out vec3 xyz, out vec3 xy_yz_xz, out vec3 xyz2, in float lvl_tree)
{
	volume = 0.0;
	xyz2 = vec3(0);
	xy_yz_xz = vec3(0);
	xyz = vec3(0);

	float r = u_curv_radius;
	
	int l = 0;
	float step = pow(2, l);
	
	int nb_probe = 0;
	float size_obj = u_size_tex;
	for(float i=0; i<r; i+=step)
	for(float j=0; j<r; j+=step)
	for(float k=0; k<r; k+=step)
	{
		vec3 probe = vec3(i + step/2.0, j + step/2.0, k + step/2.0);
		if ((length(probe) < r))
		{
			probe /= size_obj;
			
			vec3 p = vertex_position + probe;
			volume += textureLod(densities, p, l).r * (step*step*step);
			xyz += textureLod(u_xyz_tex, p, l).rgb;
			xyz2 += textureLod(u_xyz2_tex, p, l).rgb;
			xy_yz_xz += textureLod(u_xy_yz_xz_tex, p, l).rgb;
			
			probe.x *= -1;
			p = vertex_position + probe;
			volume += textureLod(densities, p, l).r * (step*step*step);
			xyz += textureLod(u_xyz_tex, p, l).rgb;
			xyz2 += textureLod(u_xyz2_tex, p, l).rgb;
			xy_yz_xz += textureLod(u_xy_yz_xz_tex, p, l).rgb;
			probe.x *= -1;
			
			probe.y *= -1;
			p = vertex_position + probe;
			volume += textureLod(densities, p, l).r * (step*step*step);
			xyz += textureLod(u_xyz_tex, p, l).rgb;
			xyz2 += textureLod(u_xyz2_tex, p, l).rgb;
			xy_yz_xz += textureLod(u_xy_yz_xz_tex, p, l).rgb;
			
			probe.x *=-1;
			p = vertex_position + probe;
			volume += textureLod(densities, p, l).r * (step*step*step);
			xyz += textureLod(u_xyz_tex, p, l).rgb;
			xyz2 += textureLod(u_xyz2_tex, p, l).rgb;
			xy_yz_xz += textureLod(u_xy_yz_xz_tex, p, l).rgb;
			probe.y *= -1;
			probe.x *= -1;
			
			probe.z *= -1;
			p = vertex_position + probe;
			volume += textureLod(densities, p, l).r * (step*step*step);
			xyz += textureLod(u_xyz_tex, p, l).rgb;
			xyz2 += textureLod(u_xyz2_tex, p, l).rgb;
			xy_yz_xz += textureLod(u_xy_yz_xz_tex, p, l).rgb;
			
			probe.x *= -1;
			p = vertex_position + probe;
			volume += textureLod(densities, p, l).r * (step*step*step);
			xyz += textureLod(u_xyz_tex, p, l).rgb;
			xyz2 += textureLod(u_xyz2_tex, p, l).rgb;
			xy_yz_xz += textureLod(u_xy_yz_xz_tex, p, l).rgb;
			probe.x *= -1;
			
			probe.y *= -1;
			p = vertex_position + probe;
			volume += textureLod(densities, p, l).r * (step*step*step);
			xyz += textureLod(u_xyz_tex, p, l).rgb;
			xyz2 += textureLod(u_xyz2_tex, p, l).rgb;
			xy_yz_xz += textureLod(u_xy_yz_xz_tex, p, l).rgb;
			
			probe.x *=-1;
			p = vertex_position + probe;
			volume += textureLod(densities, p, l).r * (step*step*step);
			xyz += textureLod(u_xyz_tex, p, l).rgb;
			xyz2 += textureLod(u_xyz2_tex, p, l).rgb;
			xy_yz_xz += textureLod(u_xy_yz_xz_tex, p, l).rgb;
			probe.y *= -1;
			probe.x *= -1;
		}
		
		nb_probe += 8;
	}
	
	return nb_probe;
}

void getEigenValuesVectors ( in float mat_data[3][3], out float vectors[3][3], out float values[3] )
{ 
	vec3 e = vec3(0);

    int dimension = 3;
	int dimensionMinusOne = 2;
	
	for( int j = 0; j < dimension; ++j )
		values[ j ] =  mat_data[dimensionMinusOne][ j ];

	// Householder reduction to tridiagonal form.
	for( int i = dimensionMinusOne; i > 0 && i <= dimensionMinusOne; --i )
    {
      // Scale to avoid under/overflow.
      float scale = 0.0;
      float h =  0.0;
      for( int k = 0; k < i; ++k )
      {
        scale += abs( values[ k ] );
      }

      if( scale ==  0.0 )
      {
        e[ i ] = values[ i - 1 ];

        for( int j = 0; j < i; ++j )
        {
            values[ j ] = mat_data[ ( i - 1 ) ] [  j  ];
            mat_data[ i ][ j ] = 0.0;
            mat_data[ j ][ i ] = 0.0;
        }
      }
      else
      {
        // Generate Householder vector.
        for ( int k = 0; k < i; ++k )
        {
            values[ k ] /= scale;
          h += values[ k ] * values[ k ];
        }

        float f = values[ i - 1 ];
        float g = sqrt( h );

        if ( f >  0.0 )
        {
          g = -g;
        }

        e[ i ] = scale * g;
        h -= f * g;
         values[ i - 1 ] = f - g;

        for ( int j = 0; j < i; ++j)
        {
          e[ j ] =  0.0;
        }

        // Apply similarity transformation to remaining columns.
        for ( int j = 0; j < i; ++j )
        {
          f = values[ j ];
             mat_data[ j ][ i ] = f;
          g = e[ j ] +  mat_data[ j ][ j ] * f;

          for ( int k = j + 1; k <= i - 1; ++k )
          {
            g +=  mat_data[ k ][ j ] * values[ k ];
            e[ k ] +=  mat_data[ k ][ j ] * f;
          }

          e[ j ] = g;
        }

        f = 0.0;
        for ( int j = 0; j < i; ++j )
        {
          e[ j ] /= h;
          f += e[ j ] * values[ j ];
        }

        float hh = f / ( h + h );

        for ( int j = 0; j < i; ++j )
        {
          e[ j ] -= hh * values[ j ];
        }

        for ( int j = 0; j < i; ++j )
        {
          f = values[ j ];
          g = e[ j ];

          for ( int k = j; k <= i - 1; ++k )
          {
                 mat_data[ k ][ j ] =   mat_data[ k ][ j ] - ( f * e[ k ] + g * values[ k ] );
          }

            values[ j ] =  mat_data[ i - 1][ j ];
            mat_data[ i ][ j ] = 0.0;
        }
      }
      values[ i ] = h;
    }

    // Accumulate transformations.
    for ( int i = 0; i < dimensionMinusOne; ++i )
    {
       mat_data[dimensionMinusOne][ i ] =  mat_data[ i ][ i ];
       mat_data[ i ][ i ] = 1.0;
      float h = values[ i + 1 ];

      if ( h != 0.0 )
      {
        for ( int k = 0; k <= i; ++k )
        {
            values[ k ] =  mat_data[ k ][ i + 1 ] / h;
        }

        for ( int j = 0; j <= i; ++j )
        {
          float g = 0.0;

          for ( int k = 0; k <= i; ++k )
          {
            g +=  mat_data[ k ][ i + 1 ] *  mat_data[ k ][ j ];
          }

          for ( int k = 0; k <= i; ++k )
          {
                 mat_data[ k ][ j ] =   mat_data[k][ j ] - ( g * values[ k ] );
          }
        }
      }
      for ( int k = 0; k <= i; ++k )
      {
             mat_data[ k ][ i + 1 ] =  0.0;
      }
    }

    for ( int j = 0; j < dimension; ++j )
    {
        values[ j ] =  mat_data[ dimensionMinusOne ][ j ];
       mat_data[ dimensionMinusOne ][ j ] = 0.0;
    }

     mat_data[ dimensionMinusOne ][ dimensionMinusOne ] =  1.0;
    e[ 0 ] =  0.0;
    
	for ( int i = 1; i < dimension; ++i )
		e[ i - 1 ] = e[ i ];

	e[ dimensionMinusOne ] = 0.0;
	
	float f = float( 0.0 );
	float tst1 = float( 0.0 );
	float eps = float( pow( 2.0, -52.0 ));
	for( int l = 0; l < dimension; ++l )
    {
      // Find small subdiagonal element
      tst1 = float( max( tst1, abs ( values[ l ] ) + abs( e[ l ] )));
      int m = l;
      while ( m < dimension )
        {
          if ( abs ( e[ m ] ) <= eps * tst1 ) break;
          ++m;
        }

      // If m == l, d[l] is an eigenvalue,
      // otherwise, iterate.
      if( m > l && l<2 )
        {
          int iter = 0;
          do
            {
              ++iter;  // (Could check iteration count here.)
              // Compute implicit shift
              float g = values[ l ];
              float p = ( values[ l + 1 ] - g ) / ( float( 2.0 ) * e[ l ] );
              float r = float( sqrt ( p * p + float( 1.0 ) * float( 1.0 )));
              if( p < 0 ) r = -r;
              values[ l ] = e[ l ] / ( p + r );
              values[ l + 1 ] = e[ l ] * ( p + r );
              float dl1 = values[ l + 1 ];
              float h = g - values[ l ];
              for( int i = l + 2; i < dimension; ++i )
                values[ i ] -= h;
              f = f + h;

              // Implicit QL transformation.
              p = values[ m ];
              float c = float( 1.0 );
              float c2 = c;
              float c3 = c;
              float el1 = e[ l + 1 ];
              float s = float( 0.0 );
              float s2 = float( 0.0 );
              for ( int i = m - 1; i >= l && i <= m - 1; --i )
                {
                  c3 = c2;
                  c2 = c;
                  s2 = s;
                  g = c * e[ i ];
                  h = c * p;
                  r = float( sqrt ( p * p + e[ i ] * e[ i ] ));
                  e[ i + 1 ] = s * r;
                  s = e[ i ] / r;
                  c = p / r;
                  p = c * values[ i ] - s * g;
                  values[ i + 1 ] = h + s * ( c * g + s * values[ i ] );

                  // Accumulate transformation.
                  for( int k = 0; k < dimension; ++k )
                    {
                      h =  mat_data[ k ][ i + 1 ];
                       mat_data[ k ][ i + 1 ] =  ( s *  mat_data[ k ][ i ] + c * h );
                       mat_data[ k ][ i ] = ( c *  mat_data[ k ][ i ] - s * h );
                    }
                }
              
              p = - s * s2 * c3 * el1 * e[ l ] / dl1;
              e[ l ] = s * p;
              values[ l ] = c * p;
              // Check for convergence.
            }
          while ( abs ( e[ l ] ) > eps * tst1 && iter < 30);
        }
      values[ l ] = values[ l ] + f;
      e[ l ] = float( 0.0 );
    }
  
  // Sort eigenvalues and corresponding vectors.
  for ( int i = 0; i < dimensionMinusOne; ++i )
    {
      int k = i;
      float p = values[ i ];
      
      for ( int j = i + 1; j < dimension; ++j )
        {
          if ( values[ j ] < p )
            {
              k = j;
              p = values[ j ];
            }
        }
      if ( k != i )
        {
            values[ k ] = values[ i ];
          values[ i ] = p;
          for ( int j = 0; j < dimension; ++j )
            {
              p =  mat_data[ j ][ i ];
               mat_data[ j ][ i ] =  mat_data[ j ][ k ];
               mat_data[ j ][ k ] = p;
            }
        }
    }
    
    for(int i=0; i<3; i++)
    for(int j=0; j<3; j++)
		vectors[i][j] = mat_data[i][j];
}


void computeK1K2(float volume, float r, 
				 vec3 xyz2, vec3 xy_yz_xz, vec3 xyz,
				 out vec3 minDir, out vec3 maxDir, out vec3 n, out vec3 val, out float k1, out float k2)
{	
  float eigenvectors[3][3];
  float eigenvalues[3];
  float curvmat[3][3];

  if (volume > 0.01 && r >= 1.0)
  {
    
    float covxy = xy_yz_xz.x - (xyz.x*xyz.y/volume);
    float covyz = xy_yz_xz.y - (xyz.y*xyz.z/volume);
    float covxz = xy_yz_xz.z - (xyz.x*xyz.z/volume);
    
    
    //volume = volume;
    curvmat[0][0] = xyz2.x - ((xyz.x*xyz.x)/(volume)); 
    curvmat[0][1] = covxy;  
    curvmat[0][2] = covxz;
    
    curvmat[1][0] = covxy; 
    curvmat[1][1] = xyz2.y - (xyz.y*xyz.y/volume);
    curvmat[1][2] = covyz;
    
    curvmat[2][0] = covxz;
    curvmat[2][1] = covyz;
    curvmat[2][2] = xyz2.z - (xyz.z*xyz.z/volume);

    getEigenValuesVectors( curvmat, eigenvectors, eigenvalues );
    
    n = vec3( eigenvectors[0][0], eigenvectors[1][0], eigenvectors[2][0] );
    minDir = vec3( eigenvectors[0][1], eigenvectors[1][1], eigenvectors[2][1] );
    maxDir = vec3( eigenvectors[0][2], eigenvectors[1][2], eigenvectors[2][2] );
    
    float l1 = eigenvalues[1];
    float l2 = eigenvalues[2];
    
    float pi = 3.14159;
    float r6 = r*r*r*r*r*r;
    k1 = (6.0/(pi*r6))*(l2 - 3.0*l1) + (8.0/(5.0*r));
    k2 = (6.0/(pi*r6))*(l1 - 3.0*l2) + (8.0/(5.0*r));
    
    val = vec3( eigenvalues[0], eigenvalues[1], eigenvalues[2] );
  }
  else
  {
	n = vec3( 0 );
    minDir = vec3( 0 );
    maxDir = vec3( 0 );

    k1 = 0;
    k2 = 0;
    
    val = vec3( 0 );
  }
}

void main( )
{
    vec3 vertex_position = position.xyz;
	vec3 geometry_position = vertex_position;
	/*curvature from regular integration*/
	float volume = 0.0;
	vec3 xyz2 = vec3(0);
	vec3 xy_yz_xz = vec3(0);
	vec3 xyz = vec3(0);
	
	vec3 eigenvalues;

	float volume_approx = 0.0;
	int nb_probe = getVolumeMoments(vertex_position, volume, xyz, xy_yz_xz, xyz2, u_lvl);

	float k1;
	float k2;
	vec3 curv_dir_min = vec3(0, 0, 1);
	vec3 curv_dir_max = vec3(0, 0, 1);
	vec3 curv_normale = vec3(0);
	
	computeK1K2(volume, u_curv_radius,
				xyz2, xy_yz_xz, xyz,
				curv_dir_min, curv_dir_max, curv_normale, eigenvalues, k1, k2);
	
	geometry_normale = curv_normale;
	
	float r = u_curv_radius;
	float fact83r = 8.0/(3.0*r);
	float fact4pir4 = 4.0 / (3.14159*r*r*r*r);
	float curvature = fact83r - fact4pir4*volume;
	
	curv_value = 0;
	if(u_curv_val == 1)
		curv_value = curvature;
	else if(u_curv_val == 2)
		curv_value = (k1*k2);
	else if(u_curv_val == 3)
		curv_value = k1;
	else if(u_curv_val == 4)
		curv_value = k2;
		
	geometry_k1_k2.x = curvature;
	geometry_k1_k2.y = k1;
	geometry_k1_k2.z = k2;
	geometry_min_dir = vec4(curv_dir_min, geometry_k1_k2.y);
	geometry_max_dir = vec4(curv_dir_max, geometry_k1_k2.z);
	vertex_color = vec3(1);
	
    gl_Position = vec4(position,1.0);
}
#endif