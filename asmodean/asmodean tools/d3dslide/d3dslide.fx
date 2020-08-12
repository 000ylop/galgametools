// d3dslide.fx, v1.03 2009/06/09
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

texture  image;
float    pos_x; 
float    pos_y;
float    alpha;
float    scale;
float4x4 proj_matrix : WorldViewProjection;

sampler image_sampler = sampler_state {
  Texture   = (image);
  MipFilter = LINEAR;
  MinFilter = LINEAR;
  MagFilter = LINEAR;
  
  // XXX: Tinkering with performance
  MaxMipLevel = 0;    
  AddressU    = CLAMP;
  AddressV    = CLAMP;
  AddressW    = CLAMP;
};

struct VS_INPUT {
  float3 position : POSITION;
  float2 texture0 : TEXCOORD0;
};

struct VS_OUTPUT {
  float4 hposition : POSITION;
  float2 texture0  : TEXCOORD0;
};

struct PS_OUTPUT {
  float4 color : COLOR;
};

/*------------------------------------------------------------------------
 * Vertex Shader: vs
 *
 * Description:
 *   Moves and scales the vertex.
 */
VS_OUTPUT vs(VS_INPUT IN) {
  VS_OUTPUT OUT; 

  float4 new_pos = float4(IN.position.xy * scale, IN.position.z, 1);
  new_pos = float4(new_pos.x + pos_x, new_pos.y + pos_y, new_pos.z, 1);

  OUT.hposition = mul(proj_matrix, new_pos);
  OUT.texture0  = IN.texture0;

  return OUT;
}

/*------------------------------------------------------------------------
 * Pixel Shader: ps
 *
 * Description:
 *   Renders a texture with dynamic alpha channel.
 */
PS_OUTPUT ps(VS_OUTPUT IN) {
  PS_OUTPUT OUT;

  OUT.color   = tex2D(image_sampler, IN.texture0);
  OUT.color.a = alpha;

  return OUT;
}

/*------------------------------------------------------------------------
 * Technique: show_image
 *
 * Description:
 *   Simple technique that supports moving and image around while the
 *   scale and alpha are adjusted.
 */
technique show_image {
  pass Pass0 {
    VertexShader = compile vs_1_1 vs();
    PixelShader  = compile ps_2_0 ps();
  }
}
