import web

urls = (
  '/data', 'Index'
)


class Index(object):
    def GET(self):
        #Parse Input
        form = web.input(skipit="skipit",magnitude="magnitude")
        skipit = int(form.skipit)
        magnitude = float(form.magnitude)
        print "skipit = " +str(skipit) + "   Magnitude = " + str(magnitude)
        return "assigned"
        #return "sleep"

if __name__ == "__main__":
    app = web.application(urls, globals())
    app.run()