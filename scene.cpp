#include "stdafx.h"

#include <string>
//#include <glm/gtx/vector_angle.hpp>



extern engine eng;
using namespace std;

void search(string base, quadtile* tile)
{
	for (size_t i = 0; i < 4; i++)
	{
		string ch = base + (char)(65 + i);
	}
}

int zl = 2;
string currentfocus = "";

float angleBetween(glm::vec3 a, glm::vec3 b, glm::vec3 origin = {0,0,0})
{
	glm::vec3 da = glm::normalize(a - origin);
	glm::vec3 db = glm::normalize(b - origin);
	return glm::acos(glm::dot(da, db));
}


scene::scene()
{
	//bunu texture sýnýfýndan türeme shadow map yap
	enable_dirlight = true;
	width = 1024.0f;
	height = 768.0f;
}


void scene::draw()
{
	if (cam == nullptr)
	{
		cam.reset(new qball_camera());
		earth.reset(new spheroid(6378137.0f, 6356752.3f));
	}
	GLenum err = 0;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	eng.sc->mxqueuedmeshes.lock();
	if (queuedmeshes.size() > 0)
	{
		for (auto tile : queuedmeshes)
		{
			tile->tm.reset(new texturemesh(tile->vertices, tile->normals, tile->uvs, string(tile->fname.begin(), tile->fname.end())));
			tile->tm->position = { 0.0, 0.0, 0.0 };
			eng.sc->meshes.push_back(tile->tm);
		}
		queuedmeshes.clear();
	}
	eng.sc->mxqueuedmeshes.unlock();

	
	glm::vec3 cameraPos = eng.sc->cam->getpos();
	if (zl != eng.sc->cam->zoomlevel)
	{
		float min = 90*4;
		vector<float> acc;

		quadtile* root = &earth->tiles;
		string letter = "";
		string mintile = "";
		string subtile = "";

		vector<double> angles;
		vector<double> angles1;
		//focus bul
		for (size_t i = 0; i < eng.sc->cam->zoomlevel-1; i++)
		{
			for (size_t x = 0; x < 4; x++)
			{
				char z = 65 + x;
				subtile = z;
				quadtile *t = root->gettile(subtile);

  				if (t == nullptr)
 					break;

				float diff = (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(t->corners.bottomleft)));
				diff += (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(t->corners.bottomright)));
				diff += (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(t->corners.upperleft)));
				diff += (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(t->corners.upperright)));


				float d1 = (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(t->corners.bottomleft)));
				float d2 = (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(t->corners.bottomright)));
				float d3 = (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(t->corners.upperleft)));
				float d4 = (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(t->corners.upperright)));




				normalspack corners = earth->getcornernormals(subtile);
				float diff1 = (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(corners.bottomleft)));
				diff1 += (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(corners.bottomright)));
				diff1 += (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(corners.upperleft)));
				diff1 += (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(corners.upperright)));


				float d5 = (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(corners.bottomleft)));
				float d6 = (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(corners.bottomright)));
				float d7 = (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(corners.upperleft)));
				float d8 = (180 / glm::pi<float>()) * acos(glm::dot(glm::normalize(cameraPos), glm::normalize(corners.upperright)));


				angles.push_back(diff);
				angles1.push_back(diff1);
				//we dont like negative guys and i don't know why the heck on earth mintile has the max angle 
				if (diff < min)
				{
					mintile = z;
					min = diff;
				}
			}

			letter += mintile;
			root = earth->tiles.gettile(letter);
		}

		zl = eng.sc->cam->zoomlevel;
		std::cout << letter << "-";

		//for (std::vector<shared_ptr<imesh>>::iterator it = eng.sc->meshes.begin(); it!= eng.sc->meshes.end(); ++it)
		//{
		//	if ((*it)->vboid_vertices == root->tm->vboid_vertices )
		//	{
		//		eng.sc->meshes.erase(it);
		//		break;
		//	}
		//}
		//root->init();
		//for (size_t i = 0; i < 4; i++)
		//{
		//	shared_ptr<tilerequest> tr(new tilerequest(&(*root).children[i]));
		//	earth->pool.queue(tr);


		//	//tiles.children[i].init();
		//	//for (size_t x = 0; x < 4; x++)
		//	//{
		//	//	tiles.children[i].children[x].init();
		//	//	for (size_t y = 0; y < 4; y++)
		//	//	{
		//	//		shared_ptr<tilerequest> tr(new tilerequest(&tiles.children[i].children[x].children[y]));
		//	//		pool.queue(tr);
		//	//	}
		//	//}
		//}



	}


	if (enable_dirlight)
	{
		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, dirlight.sm.fbo_depthmap);
		glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(programs["dirlightshadowdepth"]->get_id());
		programs["dirlightshadowdepth"]->setuniform("DirLightSpaceMatrix", dirlight.space);
		
		for (auto m : meshes)
		{
			programs["dirlightshadowdepth"]->setuniform("model", m->model());
			m->shadowdraw();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


	//NORMAL ÇÝZÝM
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(45.0f, width/height, 100.0f, 50000000.0f);
	glm::mat4 view = eng.sc->cam->getview();

	glBindVertexArray(vao_mesh_id);

	for (auto m : meshes)
	{
		glUseProgram(programs[m->spname()]->get_id());
		programs[m->spname()]->setuniform("viewPos", cameraPos);

		if (enable_dirlight)
		{
			programs[m->spname()]->setuniform("dirLight.direction", dirlight.direction);
			programs[m->spname()]->setuniform("dirLight.ambient", dirlight.ambient);
			programs[m->spname()]->setuniform("dirLight.diffuse", dirlight.diffuse);
			programs[m->spname()]->setuniform("dirLight.specular", dirlight.specular);
			programs[m->spname()]->setuniform("DirLightSpaceMatrix", dirlight.space);
			
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, dirlight.sm.vboid_texture);

			programs[m->spname()]->setuniform("far_plane", dirlight.far_plane);
			programs[m->spname()]->setuniform("dldepthMap", 3);
		}
	
		m->draw(view, projection);

		glUseProgram(0);
	}

	glBindVertexArray(0);


	glfwSwapBuffers(eng.window);
	glfwPollEvents();
	if (err != GL_NO_ERROR)
		cout << gluErrorString(err);
}
void scene::generate_shaders()
{
	programs.clear();

	if (plights.size() > 0)
	{
		program* sp = new program();

		//frag shader
		shader fsh(GL_FRAGMENT_SHADER);
		if (fsh.compile(plights[0].generate_fragshader(this)) == GL_FALSE)
			cout << "frag failed";

		if (sp->attach_shader(&fsh) != GL_NO_ERROR)
			cout << "frag attach failed";

		//vert shader
		shader vsh(GL_VERTEX_SHADER);
		if (vsh.compile(plights[0].generate_vertshader(this)) == GL_FALSE)
			cout << "vertex failed";

		if (sp->attach_shader(&vsh) != GL_NO_ERROR)
			cout << "vert attach failed";

		if (sp->link() != GL_NO_ERROR)
			cout << "link failed";

		programs["pointlight"] = sp;
	}
	if (plights.size() > 0)
	{
		program* sp = new program();


		//frag shader
		ifstream frag("./data/pointshadowdepth.frag");
		string source = "";
		string line = "";

		if (frag.is_open())
			while (std::getline(frag, line))
				source += line + "\n";
		else
			cout << "File is not open";

		shader fsh(GL_FRAGMENT_SHADER);
		if (fsh.compile(source) == GL_FALSE)
			cout << "frag failed";

		source = line = "";

		if (sp->attach_shader(&fsh) != GL_NO_ERROR)
			cout << "frag attach failed";

		//vert shader
		ifstream vertex("./data/pointshadowdepth.vert");

		if (vertex.is_open())
			while (std::getline(vertex, line))
				source += line + "\n";
		else
			cout << "File is not open";

		shader vsh(GL_VERTEX_SHADER);
		if (vsh.compile(source) == GL_FALSE)
			cout << "vertex failed";

		source = line = "";

		if (sp->attach_shader(&vsh) != GL_NO_ERROR)
			cout << "vert attach failed";


		//geo shader
		ifstream geo("./data/pointshadowdepth.geo");
		if (geo.is_open())
		{
			while (std::getline(geo, line))
				source += line + "\n";


			shader gsh(GL_GEOMETRY_SHADER);
			if (gsh.compile(source) == GL_FALSE)
				cout << "geo failed";

			source = line = "";

			if (sp->attach_shader(&gsh) != GL_NO_ERROR)
				cout << "geo attach failed";
		}


		if (sp->link() != GL_NO_ERROR)
			cout << "link failed";

		programs["pointshadowdepth"] = sp;
	}
	if (enable_dirlight)
	{
		program* sp = new program();

		//frag shader
		shader fsh(GL_FRAGMENT_SHADER);
		if (fsh.compile(dirlight.generate_fragshader(this)) == GL_FALSE)
			cout << "frag failed";

		if (sp->attach_shader(&fsh) != GL_NO_ERROR)
			cout << "frag attach failed";

		//vert shader
		shader vsh(GL_VERTEX_SHADER);
		if (vsh.compile(dirlight.generate_vertshader(this)) == GL_FALSE)
			cout << "vertex failed";

		if (sp->attach_shader(&vsh) != GL_NO_ERROR)
			cout << "vert attach failed";


		if (sp->link() != GL_NO_ERROR)
			cout << "link failed";

		programs["dirlightshadowdepth"] = sp;
	}


	program* sp = new program();

	////geo shader
	//shader gsh(GL_GEOMETRY_SHADER);
	//if (gsh.compile(m->generate_geoshader(this)) == GL_FALSE)
	//	cout << "geo failed";

	//if (sp->attach_shader(&gsh) != GL_NO_ERROR)
	//	cout << "geo attach failed";

	shared_ptr<texturemesh> m;
	m.reset(new texturemesh());

	//frag shader
	shader fsh(GL_FRAGMENT_SHADER);
	if (fsh.compile(m->generate_fragshader(this)) == GL_FALSE)
		cout << "frag failed";

	if (sp->attach_shader(&fsh) != GL_NO_ERROR)
		cout << "frag attach failed";

	//vert shader
	shader vsh(GL_VERTEX_SHADER);
	if (vsh.compile(m->generate_vertshader(this)) == GL_FALSE)
		cout << "vertex failed";

	if (sp->attach_shader(&vsh) != GL_NO_ERROR)
		cout << "vert attach failed";

	if (sp->link() != GL_NO_ERROR)
		cout << "link failed";

	programs[m->spname()] = sp;
}