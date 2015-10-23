#define _CRT_SECURE_NO_WARNINGS

#define S2O_IMPLEMENTATION
#include "sproutline.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static FILE * svg_begin(const char *filename, int w, int h)
{
	FILE *f = fopen(filename, "w");
	if (!f) return 0;
	fprintf(f,
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
		"<svg xmlns=\"http://www.w3.org/2000/svg\"\n"
		"\txmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:ev=\"http://www.w3.org/2001/xml-events\"\n"
		"\tversion=\"1.1\" baseProfile=\"full\"\n"
		"\twidth=\"%d\" height=\"%d\"\n"
		"\tviewBox=\"0 0 %d %d\">\n\n",
		w, h, w, h);
	return f;
}

static void svg_tinted_background_image(FILE *f, const char *filename)
{
	if (!f) return;
	fprintf(f, "\t<image x=\"-0.5\" y=\"-0.5\" width=\"100%%\" height=\"100%%\" xlink:href=\"%s\" />\n", filename);
	fprintf(f, "\t<rect width=\"10000\" height=\"10000\" style=\"fill:rgb(0,255,0);stroke-width:0;\" fill-opacity=\"0.5\" />\n");
}

static void svg_polygon(FILE *f, const s2o_point *outline, int length)
{
	int i;
	if (!f) return;
	fprintf(f, "\t<polygon points=\"");
	for (i = 0; i < length; i++)
		fprintf(f, "%d,%d ", outline[i].x, outline[i].y);
	fprintf(f, "\" style=\"fill:none;stroke:red;stroke-width:1\" />\n");
}

static void svg_points(FILE *f, const s2o_point *outline, int length)
{
	int i;
	if (!f) return;
	for (i = 0; i < length; i++)
		fprintf(f, "\t<circle cx=\"%d\" cy=\"%d\" r=\"0.6\" />\n", outline[i].x, outline[i].y);
}

static void svg_end(FILE *f)
{
	if (!f) return;
	fprintf(f, "</svg>");
	fclose(f);
}

int main(int argc, char **argv)
{
	const char *filename = "gnu_from_gnu.png";
	int w, h, n, l;
	unsigned char *rgba = stbi_load(filename, &w, &h, &n, 4);
	unsigned char *alpha = s2o_rgba_to_alpha(rgba, w, h);
	unsigned char *thresholded = s2o_alpha_to_thresholded(alpha, w, h, 90);
	unsigned char *outlined = s2o_thresholded_to_outlined(thresholded, w, h);
	free(rgba);
	free(alpha);
	free(thresholded);

	FILE *svg = svg_begin("result.svg", w, h);
	svg_tinted_background_image(svg, filename);

	s2o_point *outline = s2o_extract_outline_path(outlined, w, h, &l, 0);
	while(l)
	{
		int l0 = l;
		s2o_distance_based_path_simplification(outline, &l, 0.4f);
		printf("simplified outline: %d -> %d\n", l0, l);
		svg_polygon(svg, outline, l);
		svg_points(svg, outline, l);

		outline = s2o_extract_outline_path(outlined, w, h, &l, outline);
	};
	svg_end(svg);

	free(outline);
	free(outlined);
	return 0;
}
