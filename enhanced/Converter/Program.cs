using CodeWalker.GameFiles;
using CodeWalker.Core.Utils;
using System.Security.Cryptography;
using System.Text.Json;

class Program {
 static void Require(bool ok, string message) { if(!ok) throw new Exception(message); }
 static string Hash(byte[] b)=>Convert.ToHexString(SHA256.HashData(b)).ToLowerInvariant();
 static void Bones(Skeleton a, Skeleton b) {
  Require((a==null)==(b==null),"Skeleton presence changed"); if(a==null)return;
  Require(a.Bones.Items.Length==b.Bones.Items.Length,"Bone count changed");
  for(int i=0;i<a.Bones.Items.Length;i++) { var x=a.Bones.Items[i];var y=b.Bones.Items[i];
   Require(x.Tag==y.Tag&&x.Index==y.Index&&x.Index2==y.Index2&&x.ParentIndex==y.ParentIndex&&x.Translation==y.Translation&&x.Rotation==y.Rotation&&x.Scale==y.Scale,"Bone identity/bind pose changed");
  }
 }
 static void DrawableCheck(Drawable a,Drawable b) {
  Bones(a.Skeleton,b.Skeleton);
  Require(a.ShaderGroup.Shaders.Count==b.ShaderGroup.Shaders.Count,"Shader count changed");
  Require(b.ShaderGroup.Shaders.Count<=128,"Too many materials");
  var am=new[]{a.DrawableModels.High,a.DrawableModels.Med,a.DrawableModels.Low,a.DrawableModels.VLow};
  var bm=new[]{b.DrawableModels.High,b.DrawableModels.Med,b.DrawableModels.Low,b.DrawableModels.VLow};
  for(int l=0;l<4;l++) { Require((am[l]?.Length??0)==(bm[l]?.Length??0),"LOD count changed"); if(am[l]==null)continue;
   for(int m=0;m<am[l].Length;m++) { var x=am[l][m];var y=bm[l][m];Require(x.Geometries.Length==y.Geometries.Length&&x.BoneIndex==y.BoneIndex,"Geometry count/bone changed");
    for(int g=0;g<x.Geometries.Length;g++) {var u=x.Geometries[g];var v=y.Geometries[g];
     Require((u.BoneIds??Array.Empty<ushort>()).SequenceEqual(v.BoneIds??Array.Empty<ushort>()),"Skin palette changed");
     Require(u.IndexBuffer.Indices.SequenceEqual(v.IndexBuffer.Indices),"Triangles changed");
     Require(u.VertexData.VertexCount==v.VertexData.VertexCount,"Vertex count changed");
     for(int n=0;n<u.VertexData.VertexCount;n++)Require((u.VertexData.GetVector3(n,0)-v.VertexData.GetVector3(n,0)).Length()<1e-6,"Vertex position changed");
    }
   }
  }
 }
 static int Main(string[] args) {try {
  if(args[0]=="install-dlclist"||args[0]=="install-metadata") {
   bool metadata=args[0]=="install-metadata";
   string game=Path.GetFullPath(args[1]);
   Require(File.Exists(Path.Combine(game,"GTA5_Enhanced.exe")),"Enhanced executable missing");
   string source=Path.Combine(game,"update","update.rpf"),target=Path.Combine(game,"mods","update","update.rpf");
   Directory.CreateDirectory(Path.GetDirectoryName(target));
   if(File.Exists(target))File.Copy(target,target+".cu-backup-"+DateTime.Now.ToString("yyyyMMddHHmmss"));
   else File.Copy(source,target);
   RpfManager.IsGen9=true;GTA5Keys.LoadFromPath(game,true);
   var r=new RpfFile(target,"update.rpf");r.ScanStructure(s=>{},s=>throw new Exception(s));
   var entry=r.AllEntries.OfType<RpfFileEntry>().Single(e=>e.Name==(metadata?"gameconfig.xml":"dlclist.xml"));
   var doc=new System.Xml.XmlDocument();doc.XmlResolver=null;
   doc.LoadXml(System.Text.Encoding.UTF8.GetString(r.ExtractFile(entry)).TrimStart('\uFEFF'));
   if(metadata){
    File.WriteAllBytes(target+".cu-gameconfig-before.xml",r.ExtractFile(entry));
    var nodes=doc.SelectNodes("//PoolSizes/Entries/Item[PoolName='MetaDataStore']/PoolSize");Require(nodes.Count>0,"Metadata pool setting missing");
    foreach(System.Xml.XmlElement node in nodes){int old=int.Parse(node.GetAttribute("value"));if(old<4000)node.SetAttribute("value","4000");Console.WriteLine("Enhanced MetaDataStore "+old+" -> "+node.GetAttribute("value"));}
   }else{
   var paths=doc.SelectSingleNode("/SMandatoryPacksData/Paths");Require(paths!=null,"DLC paths missing");
   foreach(System.Xml.XmlNode n in paths.SelectNodes("Item"))if(n.InnerText.Trim().Replace('\\','/').TrimEnd('/').Equals("dlcpacks:/cuavatars",StringComparison.OrdinalIgnoreCase))paths.RemoveChild(n);
   var item=doc.CreateElement("Item");item.InnerText="dlcpacks:/cuavatars/";paths.AppendChild(item);
   }
   byte[] bytes=System.Text.Encoding.UTF8.GetBytes(doc.OuterXml);
   RpfFile.EnsureValidEncryption(r,f=>true);RpfFile.CreateFile(entry.Parent,entry.Name,bytes);
   var readback=new RpfFile(target,"update.rpf");readback.ScanStructure(s=>{},s=>throw new Exception(s));
   var check=readback.AllEntries.OfType<RpfFileEntry>().Single(e=>e.Name==entry.Name);
   Require(readback.ExtractFile(check).SequenceEqual(bytes),"DLC list readback mismatch");
   if(metadata)File.WriteAllBytes(target+".cu-gameconfig-after.xml",bytes);
   Console.WriteLine("Patched "+entry.Name+" in Enhanced mods archive; verified readback; original archive untouched.");return 0;
  }
  string input=Path.GetFullPath(args[0]),output=Path.GetFullPath(args[1]);Require(input!=output,"Separate output required");Directory.CreateDirectory(output);
  var report=new List<object>();RpfManager.IsGen9=true;
  foreach(var path in Directory.GetFiles(input).OrderBy(p=>p)) {
   string ext=Path.GetExtension(path);Require(new[]{".ydd",".yft",".ytd",".ymt",".ycd"}.Contains(ext),"Unexpected input");
   byte[] original=File.ReadAllBytes(path);var converted=Gen9Converter.TryConvert(original,ext,Console.WriteLine,Path.GetFileName(path),true,out bool changed);Require(converted!=null,"Conversion failed");
   if(ext==".ydd") {var a=new YddFile();a.Load(original);var b=new YddFile();b.Load(converted);Require(((RpfResourceFileEntry)b.RpfFileEntry).Version==159,"Not Gen9 YDD");Require(a.Drawables.Length==b.Drawables.Length,"Drawable count changed");for(int i=0;i<a.Drawables.Length;i++)DrawableCheck(a.Drawables[i],b.Drawables[i]);}
   if(ext==".yft") {var a=new YftFile();a.Load(original);var b=new YftFile();b.Load(converted);Require(((RpfResourceFileEntry)b.RpfFileEntry).Version==171,"Not Gen9 YFT");Bones(a.Fragment.Drawable.Skeleton,b.Fragment.Drawable.Skeleton);}
   if(ext==".ytd") {var a=new YtdFile();a.Load(original);var b=new YtdFile();b.Load(converted);Require(((RpfResourceFileEntry)b.RpfFileEntry).Version==5,"Not Gen9 YTD");var x=a.TextureDict.Textures.data_items;var y=b.TextureDict.Textures.data_items;Require(x.Length==y.Length,"Texture count changed");for(int i=0;i<x.Length;i++)Require(x[i].Name==y[i].Name&&x[i].Width==y[i].Width&&x[i].Height==y[i].Height,"Texture changed");}
   if(ext==".ycd"||ext==".ymt")Require(original.SequenceEqual(converted),"Shared-format resource changed");
   File.WriteAllBytes(Path.Combine(output,Path.GetFileName(path)),converted);
   report.Add(new {file=Path.GetFileName(path),changed,source_sha256=Hash(original),sha256=Hash(converted),bytes=converted.Length});
  }
  Require(report.Count==15,"Expected exactly 15 avatar resources");
  File.WriteAllText(Path.Combine(Path.GetDirectoryName(output),"conversion-report.json"),JsonSerializer.Serialize(new {target="GTA V Enhanced",live_tested=false,files=report},new JsonSerializerOptions{WriteIndented=true}));return 0;
 }catch(Exception e){Console.Error.WriteLine(e);return 1;}}
}
