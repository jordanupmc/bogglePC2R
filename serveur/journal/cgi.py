#!/usr/bin/env python
print "Content-type: text/html"
print

def genHtml(f):
    res =""
    for line in f:
        entry = line.split(' ');
        #print entry
        if entry[0] == "F\n" or entry[0] == "F":
            res+="</tbody></table></div>"
        elif entry[0] == "S":
            res+="""<div class="container">
  <h2>"""+entry[1]+" tours "+entry[2]+"""</h2><table class="table table-light table-sm table-hover"><thead><tr>
    <th>Nom</th>
    <th>Score</th>
</tr>
    </thead><tbody>"""

        elif entry[0] == "U":
            users = entry[1].split('*')
           
            del users[0]
            i=0
            for u in users:
               if i%2 == 0:
                   name=u
               else:
                   res+="<tr><td>"+name+"</td><td>"+u+"</td></tr>"
               i=i+1
    return res

file = open("journal.jou","r") 
content = genHtml(file)

html = """<!DOCTYPE html>
<html >
<head><meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.1.0/css/bootstrap.min.css">
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.0/umd/popper.min.js"></script>
  <script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.1.0/js/bootstrap.min.js"></script>
      <title>Resultats de sessions</title>
</head>
<body style="background-color: rgb(245, 244, 235);color: rgb(137, 54, 103); ">
<img src="img/logo.png" style="display: block;
    margin-left: auto;
    margin-right: auto;
    width: 24%"/>
<h1 style="text-align: center;
           font-family:Impact, Charcoal, sans-serif
">Resultats des sessions</h1><br>"""+content+"""</body>
</html>
"""
file.close

print(html)
