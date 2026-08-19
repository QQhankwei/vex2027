(()=>{
const NS='http://www.w3.org/2000/svg';
const add=(parent,tag,attributes={})=>{const node=document.createElementNS(NS,tag);for(const [key,value] of Object.entries(attributes))node.setAttribute(key,value);parent.appendChild(node);return node};

/** 共用的 V5RC Override 場地視覺層；座標為 0~100 SVG 場地比例。 */
window.drawOverrideFieldLayer=function(target){
  const group=typeof target==='string'?document.querySelector(target):target;
  if(!group||group.dataset.overrideField==='ready')return;
  group.dataset.overrideField='ready';group.classList.add('override-field-layer');

  // Alliance corners and tape lines.
  add(group,'path',{d:'M1 18V1H18 M82 1H99V18',class:'alliance-line alliance-top'});
  add(group,'path',{d:'M1 82V99H18 M82 99H99V82',class:'alliance-line alliance-bottom'});
  add(group,'path',{d:'M0 0L100 100M100 0L0 100M33 50L50 33L67 50L50 67Z',class:'field-tape'});

  // Four wall toggles and adjacent loaders.
  const walls=[{x:50,y:1,r:0,c:'red'},{x:99,y:50,r:90,c:'red'},{x:50,y:99,r:0,c:'blue'},{x:1,y:50,r:90,c:'blue'}];
  for(const wall of walls){
    const toggle=add(group,'g',{transform:`translate(${wall.x} ${wall.y}) rotate(${wall.r})`,class:`field-toggle ${wall.c}`});
    add(toggle,'rect',{x:-3.8,y:-1.1,width:7.6,height:2.2,rx:.7});add(toggle,'circle',{cx:0,cy:0,r:.75,class:'toggle-center'});
  }
  for(const [x,y] of [[1,25],[1,75],[99,25],[99,75]]){
    const loader=add(group,'g',{transform:`translate(${x} ${y})`,class:'field-loader'});
    add(loader,'rect',{x:-1.4,y:-4,width:2.8,height:8,rx:.7});add(loader,'path',{d:'M-1.3-2.5L1.3 0L-1.3 2.5'});
  }

  // AprilTag map from the published Override setup diagram.
  const tags=[[50,50,'0'],[10,33,'1'],[90,67,'1'],[90,33,'2'],[10,67,'2'],[67,8,'3'],[33,92,'3'],[33,8,'4'],[67,92,'4']];
  for(const [x,y,label] of tags){const tag=add(group,'g',{transform:`translate(${x} ${y})`,class:'april-tag'});add(tag,'rect',{x:-2.1,y:-2.1,width:4.2,height:4.2,rx:.5});const text=add(tag,'text',{x:0,y:.9,'text-anchor':'middle'});text.textContent=label}

  // Goal bodies: center tall, neutral short, and alliance goals.
  const goals=[[16.7,33.3,'neutral'],[16.7,66.7,'neutral'],[33.3,16.7,'neutral'],[66.7,16.7,'blue'],[50,50,'tall'],[33.3,83.3,'red'],[66.7,83.3,'neutral'],[83.3,33.3,'red'],[83.3,66.7,'blue']];
  for(const [x,y,type] of goals){
    const goal=add(group,'g',{transform:`translate(${x} ${y})`,class:`field-goal ${type}`});
    add(goal,'circle',{r:type==='tall'?4.2:3.5,class:'goal-base'});add(goal,'circle',{r:type==='tall'?1.35:1.15,class:'goal-post'});
    if(type==='tall')add(goal,'circle',{r:2.6,class:'goal-tier'});
  }

  // Simplified pins/cups make strategic obstacles visible without obscuring paths.
  const pieces=[
    [27,27,'red'],[73,73,'blue'],[27,73,'yellow'],[73,27,'yellow'],
    [50,15,'yellow'],[50,85,'yellow'],[15,50,'yellow'],[85,50,'yellow'],
    [38,38,'red'],[62,62,'blue'],[38,62,'yellow'],[62,38,'yellow']
  ];
  for(const [x,y,color] of pieces){const piece=add(group,'g',{transform:`translate(${x} ${y})`,class:`field-piece ${color}`});add(piece,'circle',{r:1.25});add(piece,'path',{d:'M0-2.2L.65-1L0-.5L-.65-1Z',class:'piece-pin'})}
};
document.addEventListener('DOMContentLoaded',()=>{
  window.drawOverrideFieldLayer('#goals');
  window.drawOverrideFieldLayer('#driverGoals');
});
})();
