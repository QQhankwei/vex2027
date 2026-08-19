// V5 Brain 選程式畫面的電腦端鏡像。
// 此頁只呈現真實狀態：只有 generated-auto.h 內的 Auto 標示為 READY。
(()=>{
const grid=document.querySelector('#brainAutoGrid');
if(!grid)return;
const status=document.querySelector('#selectorStatus');
const compiledName=document.querySelector('#compiledAutoName');
const selectedName=document.querySelector('#selectedAutoName');
const brainSelection=document.querySelector('#brainSelection');
let autos=[],selection={selectedAutoName:'none',compiledAutoNames:[]};

const escapeHtml=value=>String(value).replace(/[&<>"']/g,character=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[character]));
function selectionState(name){
  if(name==='none')return {className:'safe',label:'SAFE／不執行'};
  if((selection.compiledAutoNames||[]).includes(name))return {className:'ready',label:'READY／可執行'};
  return {className:'draft',label:'DRAFT／尚未編譯'};
}
function render(){
  const names=['none',...autos.map(auto=>auto.name)];
  grid.innerHTML=names.map(name=>{const state=selectionState(name),selected=name===selection.selectedAutoName;return `<button class="brain-auto-button ${state.className} ${selected?'selected':''}" data-auto-name="${escapeHtml(name)}"><b>${name==='none'?'NO AUTO／安全停用':escapeHtml(name)}</b><small>${state.label}</small></button>`}).join('');
  grid.querySelectorAll('[data-auto-name]').forEach(button=>button.onclick=()=>selectAuto(button.dataset.autoName));
  compiledName.textContent=(selection.compiledAutoNames||[]).join(' · ')||'NONE／無';
  selectedName.textContent=selection.selectedAutoName==='none'?'NO AUTO／安全停用':selection.selectedAutoName;
  brainSelection.textContent=`SELECTED／已選：${selection.selectedAutoName.toUpperCase()}`;
  const state=selectionState(selection.selectedAutoName),ready=state.className==='ready'||state.className==='safe';
  status.classList.toggle('ready',ready);
  status.innerHTML=`<b>${state.label}</b><br>${ready?(selection.selectedAutoName==='none'?'Autonomous will remain stopped.／自動階段保持停止。':'C++ Core contains this program.／C++ 核心已包含此程式。'):'This JSON is not the compiled program yet.／這份 JSON 尚不是目前編譯程式。'}`;
  document.querySelector('#runSelectedAuto').disabled=selection.selectedAutoName==='none'||!ready;
  document.dispatchEvent(new CustomEvent('vex-auto-selection-changed',{detail:{...selection}}));
}
async function selectAuto(name){
  const response=await fetch('/api/auto-selection',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name})});
  if(!response.ok){status.textContent=`SELECT ERROR／選擇失敗：${await response.text()}`;return}
  selection.selectedAutoName=name;render();
}
async function load(){
  try{
    const [autosResponse,selectionResponse]=await Promise.all([fetch('/api/autos'),fetch('/api/auto-selection')]);
    autos=(await autosResponse.json()).autos||[];selection=await selectionResponse.json();render();
  }catch(error){status.textContent=`LOAD ERROR／讀取失敗：${error.message}`}
}
function openSelectedAuto(){
  if(selection.selectedAutoName!=='none'){
    const route=document.querySelector('#route');
    if([...route.options].some(option=>option.value===selection.selectedAutoName)){route.value=selection.selectedAutoName;route.dispatchEvent(new Event('change'))}
  }
  document.querySelector('[data-page="auto"]').click();
}
document.querySelector('#openSelectedAuto').onclick=openSelectedAuto;
document.querySelector('#runSelectedAuto').onclick=()=>{openSelectedAuto();setTimeout(()=>document.querySelector('#play').click(),100)};
load();
})();
