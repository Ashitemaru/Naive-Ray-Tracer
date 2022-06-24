#include "utils/scene_parser.hpp"

#define degreesToRadians(x) ((M_PI * x) / 180.)

SceneParser::SceneParser(const char *filename) {
	group = nullptr;
	camera = nullptr;
	backgroundColor = Vector3f(0.5, 0.5, 0.5);
	numLights = 0;
	lights = nullptr;
	numMaterials = 0;
	materials = nullptr;
	currentMaterial = nullptr;

	// Parse the file
	assert(filename != nullptr);
	const char *ext = &filename[strlen(filename) - 4];

	if (strcmp(ext, ".txt") != 0) {
		printf("Wrong file name extension\n");
		exit(0);
	}

	file = fopen(filename, "r");
	if (file == nullptr) {
		printf("Cannot open scene file\n");
		exit(0);
	}

	parseFile();
	fclose(file);
	file = nullptr;

	if (numLights == 0)
		printf("WARNING: No lights specified\n");
}

SceneParser::~SceneParser() {
	delete group;
	delete camera;

	int i;
	for (i = 0; i < numMaterials; i++)
		delete materials[i];
	delete[] materials;

	for (i = 0; i < numLights; i++)
		delete lights[i];
	delete[] lights;
}

void SceneParser::parseFile() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	while (getToken(token)) {
		if (!strcmp(token, "PerspectiveCamera")) {
			parsePerspectiveCamera();
		} else if (!strcmp(token, "LensCamera")) {
			parseLensCamera();
		} else if (!strcmp(token, "Background")) {
			parseBackground();
		} else if (!strcmp(token, "Lights")) {
			parseLights();
		} else if (!strcmp(token, "Materials")) {
			parseMaterials();
		} else if (!strcmp(token, "Group")) {
			group = parseGroup();
		} else {
			printf("Unknown token in parseFile: '%s'\n", token);
			exit(0);
		}
	}
}

void SceneParser::parsePerspectiveCamera() {
	char token[MAX_PARSER_TOKEN_LENGTH];

	getToken(token);
	assert(!strcmp(token, "{"));
	getToken(token);
	assert(!strcmp(token, "center"));
	Vector3f center = readVector3f();
	getToken(token);
	assert(!strcmp(token, "direction"));
	Vector3f direction = readVector3f();
	getToken(token);
	assert(!strcmp(token, "up"));
	Vector3f up = readVector3f();
	getToken(token);
	assert(!strcmp(token, "angle"));
	double angle_degrees = readDouble();
	double angle_radians = degreesToRadians(angle_degrees);
	getToken(token);
	assert(!strcmp(token, "width"));
	int width = readInt();
	getToken(token);
	assert(!strcmp(token, "height"));
	int height = readInt();
	getToken(token);
	assert(!strcmp(token, "gamma"));
	int gamma = readDouble();
	getToken(token);
	assert(!strcmp(token, "}"));
	camera = new PerspectiveCamera(center, direction, up, width, height, gamma, angle_radians);
}

void SceneParser::parseLensCamera() {
	char token[MAX_PARSER_TOKEN_LENGTH];

	getToken(token);
	assert(!strcmp(token, "{"));
	getToken(token);
	assert(!strcmp(token, "center"));
	Vector3f center = readVector3f();
	getToken(token);
	assert(!strcmp(token, "direction"));
	Vector3f direction = readVector3f();
	getToken(token);
	assert(!strcmp(token, "up"));
	Vector3f up = readVector3f();
	getToken(token);
	assert(!strcmp(token, "angle"));
	double angle_degrees = readDouble();
	double angle_radians = degreesToRadians(angle_degrees);
	getToken(token);
	assert(!strcmp(token, "width"));
	int width = readInt();
	getToken(token);
	assert(!strcmp(token, "height"));
	int height = readInt();
	getToken(token);
	assert(!strcmp(token, "gamma"));
	double gamma = readDouble();
	getToken(token);
	assert(!strcmp(token, "aperture"));
	double aperture = readDouble();
	getToken(token);
	assert(!strcmp(token, "focal"));
	double focal = readDouble();
	getToken(token);
	assert(!strcmp(token, "}"));
	camera = new LenCamera(center, direction, up, width, height, gamma, angle_radians, aperture, focal);
}

void SceneParser::parseBackground() {
	char token[MAX_PARSER_TOKEN_LENGTH];

	getToken(token);
	assert(!strcmp(token, "{"));
	while (true) {
		getToken(token);
		if (!strcmp(token, "}")) {
			break;
		} else if (!strcmp(token, "color")) {
			backgroundColor = readVector3f();
		} else if (!strcmp(token, "ambient")) {
			ambientColor = readVector3f();
		} else {
			printf("Unknown token in parseBackground: '%s'\n", token);
			assert(0);
		}
	}
}

void SceneParser::parseLights() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	getToken(token);
	assert(!strcmp(token, "{"));

	getToken(token);
	assert(!strcmp(token, "numLights"));
	numLights = readInt();
	lights = new Light *[numLights];

	int count = 0;
	while (numLights > count) {
		getToken(token);
		if (strcmp(token, "AreaLight") == 0) {
			lights[count] = parseAreaLight();
		} else if (strcmp(token, "PointLight") == 0) {
			lights[count] = parsePointLight();
		} else if (strcmp(token, "DirectedPointLight") == 0) {
			lights[count] = parseDirectedPointLight();
		} else {
			printf("Unknown token in parseLight: '%s'\n", token);
			exit(0);
		}
		count++;
	}
	getToken(token);
	assert(!strcmp(token, "}"));
}

AreaLight *SceneParser::parseAreaLight() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	getToken(token);
	assert(!strcmp(token, "{"));
	Object3D* object = parseGroup();
	getToken(token);
	assert(!strcmp(token, "power"));
	Vector3f power = readVector3f();
	getToken(token);
	assert(!strcmp(token, "}"));
	return new AreaLight(object, power);
}

PointLight *SceneParser::parsePointLight() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	getToken(token);
	assert(!strcmp(token, "{"));
	getToken(token);
	assert(!strcmp(token, "position"));
	Vector3f position = readVector3f();
	getToken(token);
	assert(!strcmp(token, "power"));
	Vector3f power = readVector3f();
	getToken(token);
	assert(!strcmp(token, "}"));
	return new PointLight(position, power);
}

DirectedPointLight *SceneParser::parseDirectedPointLight() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	getToken(token);
	assert(!strcmp(token, "{"));
	getToken(token);
	assert(!strcmp(token, "position"));
	Vector3f position = readVector3f();
	getToken(token);
	assert(!strcmp(token, "direction"));
	Vector3f dir = readVector3f();
	getToken(token);
	assert(!strcmp(token, "angle"));
	double angle_degrees = readDouble();
	double angle_radians = degreesToRadians(angle_degrees);
	getToken(token);
	assert(!strcmp(token, "power"));
	Vector3f power = readVector3f();
	getToken(token);
	assert(!strcmp(token, "}"));
	return new DirectedPointLight(position, dir, power, angle_radians);
}

void SceneParser::parseMaterials() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	getToken(token);
	assert(!strcmp(token, "{"));

	getToken(token);
	assert(!strcmp(token, "numMaterials"));
	numMaterials = readInt();
	materials = new Material *[numMaterials];

	int count = 0;
	while (numMaterials > count) {
		getToken(token);
		if (!strcmp(token, "GenericMaterial")) {
			materials[count] = parseGenericMaterial();
		} else if (!strcmp(token, "PhongMaterial")) {
			materials[count] = parsePhongMaterial();
		} else if (!strcmp(token, "LambertMaterial")) {
			materials[count] = parseLambertMaterial();
		} else if (!strcmp(token, "MirrorMaterial")) {
			materials[count] = parseSpecularMaterial();
		} else if (!strcmp(token, "TransparentMaterial")) {
			materials[count] = parseTransparentMaterial();
		} else {
			printf("Unknown token in parseMaterial: '%s'\n", token);
			exit(0);
		}
		count++;
	}
	getToken(token);
	assert(!strcmp(token, "}"));
}


Material *SceneParser::parseGenericMaterial() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	char filename[MAX_PARSER_TOKEN_LENGTH];
	bool hasTexture = false;
	Vector3f Ka(1, 1, 1), Kd(1, 1, 1), Ks(1, 1, 1);
	double Ns = 0., Ni = 1., d = 1.;
	int illum = -1; 
	getToken(token);
	assert(!strcmp(token, "{"));
	while (true) {
		getToken(token);
		if (strcmp(token, "Ka") == 0) {
			Ka = readVector3f();
		} else if (strcmp(token, "Kd") == 0) {
			Kd = readVector3f();
		} else if (strcmp(token, "Ks") == 0) {
			Ks = readVector3f();
		} else if (strcmp(token, "Ns") == 0) {
			Ns = readDouble();
		} else if (strcmp(token, "Ni") == 0) {
			Ni = readDouble();
		} else if (strcmp(token, "d") == 0) {
			d = readDouble();
		} else if (strcmp(token, "illum") == 0) {
			illum = readInt();
		} else if (strcmp(token, "texture") == 0 || strcmp(token, "map_Kd") == 0) {
			getToken(filename);
			hasTexture = true;
		} else {
			assert(!strcmp(token, "}"));
			break;
		}
	}
	
	if (illum == 0 || illum == 1)
		return hasTexture ? new Lambert(Kd, filename) : new Lambert(Kd);
	else if (illum == 2)
		return hasTexture ? new Phong(Kd, Ks, Ns, filename) : new Phong(Kd, Ks, Ns);
	else if (illum == 5)
		return hasTexture ? new Specular(Ks, filename) : new Specular(Ks);
	else if (illum == 7)
		return hasTexture ?new Transparent(Ks, Ni, filename) : new Transparent(Ks, Ni);
	else
		return hasTexture ? new General(Ka, Kd, Ks, Ns, Ni, d, filename) : new General(Ka, Kd, Ks, Ns, Ni, d);
}


Lambert *SceneParser::parseLambertMaterial() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	char filename[MAX_PARSER_TOKEN_LENGTH];
	bool hasTexture = false;
	Vector3f color(1, 1, 1);
	getToken(token);
	assert(!strcmp(token, "{"));
	while (true) {
		getToken(token);
		if (strcmp(token, "color") == 0) {
			color = readVector3f();
		} else if (strcmp(token, "texture") == 0) {
			getToken(filename);
			hasTexture = true;
		} else {
			assert(!strcmp(token, "}"));
			break;
		}
	}
	Lambert *answer;
	if (hasTexture)
		answer = new Lambert(color, filename);
	else
		answer = new Lambert(color);
	return answer;
}

Phong *SceneParser::parsePhongMaterial() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	char filename[MAX_PARSER_TOKEN_LENGTH];
	bool hasTexture = false;
	Vector3f diffuseColor(1, 1, 1), specularColor(0, 0, 0);
	double shininess = 0;
	getToken(token);
	assert(!strcmp(token, "{"));
	while (true) {
		getToken(token);
		if (strcmp(token, "diffuseColor") == 0) {
			diffuseColor = readVector3f();
		} else if (strcmp(token, "specularColor") == 0) {
			specularColor = readVector3f();
		} else if (strcmp(token, "shininess") == 0) {
			shininess = readDouble();
		} else if (strcmp(token, "texture") == 0) {
			getToken(filename);
			hasTexture = true;
		} else {
			assert(!strcmp(token, "}"));
			break;
		}
	}
	Phong *answer;
	if (hasTexture)
		answer = new Phong(diffuseColor, specularColor, shininess, filename);
	else
		answer = new Phong(diffuseColor, specularColor, shininess);
	return answer;
}

Specular *SceneParser::parseSpecularMaterial() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	char filename[MAX_PARSER_TOKEN_LENGTH];
	bool hasTexture = false;
	Vector3f color(1, 1, 1);
	getToken(token);
	assert(!strcmp(token, "{"));
	while (true) {
		getToken(token);
		if (strcmp(token, "color") == 0) {
			color = readVector3f();
		} else if (strcmp(token, "texture") == 0) {
			getToken(filename);
			hasTexture = true;
		} else {
			assert(!strcmp(token, "}"));
			break;
		}
	}
	Specular *answer;
	if (hasTexture) 
		answer = new Specular(color, filename);
	else
		answer = new Specular(color);
	return answer;
}

Transparent *SceneParser::parseTransparentMaterial() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	char filename[MAX_PARSER_TOKEN_LENGTH];
	bool hasTexture = false;
	Vector3f color(1, 1, 1);
	double ior = 1.;
	getToken(token);
	assert(!strcmp(token, "{"));
	while (true) {
		getToken(token);
		if (strcmp(token, "color") == 0) {
			color = readVector3f();
		} else if (strcmp(token, "index") == 0) {
			ior = readDouble();
		} else if (strcmp(token, "texture") == 0) {
			getToken(filename);
			hasTexture = true;
		} else {
			assert(!strcmp(token, "}"));
			break;
		}
	}
	Transparent *answer;
	if (hasTexture) 
		answer = new Transparent(color, ior, filename);
	else
		answer = new Transparent(color, ior);
	return answer;
}

Object3D *SceneParser::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) {
	Object3D *answer = nullptr;
	if (!strcmp(token, "Group")) {
		answer = (Object3D *)parseGroup();
	} else if (!strcmp(token, "Sphere")) {
		answer = (Object3D *)parseSphere();
	} else if (!strcmp(token, "Plane")) {
		answer = (Object3D *)parsePlane();
	} else if (!strcmp(token, "Rectangle")) {
		answer = (Object3D *)parseRectangle();
	} else if (!strcmp(token, "Triangle")) {
		answer = (Object3D *)parseTriangle();
	} else if (!strcmp(token, "TriangleMesh")) {
		answer = (Object3D *)parseTriangleMesh();
	} else if (!strcmp(token, "Transform")) {
		answer = (Object3D *)parseTransform();
	} else {
		printf("Unknown token in parseObject: '%s'\n", token);
		exit(0);
	}
	return answer;
}

Group *SceneParser::parseGroup() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	getToken(token);
	assert(!strcmp(token, "{"));

	getToken(token);
	assert(!strcmp(token, "numObjects"));
	int num_objects = readInt();

	auto *answer = new Group(num_objects);

	int count = 0;
	while (num_objects > count) {
		getToken(token);
		if (!strcmp(token, "MaterialIndex")) {
			int index = readInt();
			assert(index >= 0 && index <= getNumMaterials());
			currentMaterial = getMaterial(index);
		} else {
			Object3D *object = parseObject(token);
			assert(object != nullptr);
			answer->append(object);

			count++;
		}
	}
	getToken(token);
	assert(!strcmp(token, "}"));

	return answer;
}

Sphere *SceneParser::parseSphere() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	getToken(token);
	assert(!strcmp(token, "{"));
	getToken(token);
	assert(!strcmp(token, "center"));
	Vector3f center = readVector3f();
	getToken(token);
	assert(!strcmp(token, "radius"));
	double radius = readDouble();
	getToken(token);
	assert(!strcmp(token, "}"));
	assert(currentMaterial != nullptr);
	return new Sphere(center, radius, currentMaterial);
}

Plane *SceneParser::parsePlane() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	bool hasTexture = false;
	Vector3f e1, e2, origin;
	getToken(token);
	assert(!strcmp(token, "{"));
	getToken(token);
	assert(!strcmp(token, "normal"));
	Vector3f normal = readVector3f();
	getToken(token);
	assert(!strcmp(token, "offset"));
	double offset = readDouble();
	getToken(token);
	if (!strcmp(token, "e1")) {
		hasTexture = true;
		e1 = readVector3f();
		getToken(token);
		assert(!strcmp(token, "e2"));
		e2 = readVector3f();
		getToken(token);
		assert(!strcmp(token, "origin"));
		origin = readVector3f();
		getToken(token);
	}
	assert(!strcmp(token, "}"));
	assert(currentMaterial != nullptr);
	Plane* p = new Plane(normal, offset, currentMaterial);
	if (hasTexture)
		p->addTexture(e1, e2, origin);
	return p;
}

Rectangle *SceneParser::parseRectangle() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	getToken(token);
	assert(!strcmp(token, "{"));
	getToken(token);
	assert(!strcmp(token, "a"));
	Vector3f a = readVector3f();
	getToken(token);
	assert(!strcmp(token, "b"));
	Vector3f b = readVector3f();
	getToken(token);
	assert(!strcmp(token, "}"));
	assert(currentMaterial != nullptr);
	return new Rectangle(a, b, currentMaterial);
}

Triangle *SceneParser::parseTriangle() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	getToken(token);
	assert(!strcmp(token, "{"));
	getToken(token);
	assert(!strcmp(token, "vertex0"));
	Vector3f v0 = readVector3f();
	getToken(token);
	assert(!strcmp(token, "vertex1"));
	Vector3f v1 = readVector3f();
	getToken(token);
	assert(!strcmp(token, "vertex2"));
	Vector3f v2 = readVector3f();
	getToken(token);
	assert(!strcmp(token, "}"));
	assert(currentMaterial != nullptr);
	return new Triangle(v0, v1, v2, currentMaterial);
}

Mesh *SceneParser::parseTriangleMesh() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	char filename[MAX_PARSER_TOKEN_LENGTH];

	getToken(token);
	assert(!strcmp(token, "{"));
	getToken(token);
	assert(!strcmp(token, "obj_file"));
	getToken(filename);
	getToken(token);
	assert(!strcmp(token, "}"));
	const char *ext = &filename[strlen(filename) - 4];
	assert(!strcmp(ext, ".obj"));
	Mesh *answer = new Mesh(filename, currentMaterial);

	return answer;
}

Transform *SceneParser::parseTransform() {
	char token[MAX_PARSER_TOKEN_LENGTH];
	Matrix4f matrix = Matrix4f::identity();
	Object3D *object = nullptr;
	getToken(token);
	assert(!strcmp(token, "{"));

	getToken(token);

	while (true) {
		if (!strcmp(token, "Scale")) {
			Vector3f s = readVector3f();
			matrix = matrix * Matrix4f::scaling(s[0], s[1], s[2]);
		} else if (!strcmp(token, "UniformScale")) {
			double s = readDouble();
			matrix = matrix * Matrix4f::uniformScaling(s);
		} else if (!strcmp(token, "Translate")) {
			matrix = matrix * Matrix4f::translation(readVector3f());
		} else if (!strcmp(token, "XRotate")) {
			matrix = matrix * Matrix4f::rotateX(degreesToRadians(readDouble()));
		} else if (!strcmp(token, "YRotate")) {
			matrix = matrix * Matrix4f::rotateY(degreesToRadians(readDouble()));
		} else if (!strcmp(token, "ZRotate")) {
			matrix = matrix * Matrix4f::rotateZ(degreesToRadians(readDouble()));
		} else if (!strcmp(token, "Rotate")) {
			getToken(token);
			assert(!strcmp(token, "{"));
			Vector3f axis = readVector3f();
			double degrees = readDouble();
			double radians = degreesToRadians(degrees);
			matrix = matrix * Matrix4f::rotation(axis, radians);
			getToken(token);
			assert(!strcmp(token, "}"));
		} else if (!strcmp(token, "Matrix4f")) {
			Matrix4f matrix2 = Matrix4f::identity();
			getToken(token);
			assert(!strcmp(token, "{"));
			for (int j = 0; j < 4; j++)
				for (int i = 0; i < 4; i++) {
					double v = readDouble();
					matrix2(i, j) = v;
				}
			getToken(token);
			assert(!strcmp(token, "}"));
			matrix = matrix2 * matrix;
		} else {
			object = parseObject(token);
			break;
		}
		getToken(token);
	}

	assert(object != nullptr);
	getToken(token);
	assert(!strcmp(token, "}"));
	return new Transform(matrix, object);
}

int SceneParser::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
	assert(file != nullptr);
	int success = fscanf(file, "%s ", token);
	if (success == EOF) {
		token[0] = '\0';
		return 0;
	}
	return 1;
}

Vector3f SceneParser::readVector3f() {
	double x, y, z;
	int count = fscanf(file, "%lf %lf %lf", &x, &y, &z);
	if (count != 3) {
		printf("Error trying to read 3 doubles to make a Vector3f\n");
		assert(0);
	}
	return Vector3f(x, y, z);
}

double SceneParser::readDouble() {
	double answer;
	int count = fscanf(file, "%lf", &answer);
	if (count != 1) {
		printf("Error trying to read 1 double\n");
		assert(0);
	}
	return answer;
}

int SceneParser::readInt() {
	int answer;
	int count = fscanf(file, "%d", &answer);
	if (count != 1) {
		printf("Error trying to read 1 int\n");
		assert(0);
	}
	return answer;
}
