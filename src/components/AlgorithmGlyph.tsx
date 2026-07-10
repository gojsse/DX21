import type { CSSProperties } from 'react'
import { ALGS } from '../state/seed'
import type { AlgAccent } from '../theme/themes'

/**
 * Algorithm routing glyph — ported from the handoff's gl() SVG generator.
 * Carriers are filled, modulators outlined; OP4 gets a feedback bracket.
 * When `selected`, the operator blocks breathe via the opPulse keyframe.
 */
export function AlgorithmGlyph({ index, selected, A }: { index: number; selected: boolean; A: AlgAccent }) {
  const G = ALGS[index]
  const bw = 13, bh = 10, gx = 9, gy = 8, pad = 4, topPad = 5
  const ns = Object.entries(G.nodes)
  const maxX = Math.max(...ns.map(([, v]) => v[0]))
  const maxY = Math.max(...ns.map(([, v]) => v[1]))
  const W = pad * 2 + maxX * (bw + gx) + bw + 7
  const H = pad + topPad + maxY * (bh + gy) + bh + pad
  const pos: Record<string, { x: number; y: number }> = {}
  ns.forEach(([k, v]) => { pos[k] = { x: pad + v[0] * (bw + gx), y: pad + topPad + (maxY - v[1]) * (bh + gy) } })

  const stroke = selected ? A.ac : A.base
  const kids: JSX.Element[] = []

  G.edges.forEach((ed, j) => {
    kids.push(<line key={'e' + j} x1={pos[ed[0]].x + bw / 2} y1={pos[ed[0]].y + bh} x2={pos[ed[1]].x + bw / 2} y2={pos[ed[1]].y} stroke={stroke} strokeWidth={1.2} opacity={0.85} />)
  })

  const p4 = pos[4]
  kids.push(<path key="fb" d={`M ${p4.x + bw} ${p4.y + bh / 2} h 4 V ${p4.y - 3} H ${p4.x + bw / 2}`} fill="none" stroke={stroke} strokeWidth={1} opacity={0.7} />)

  ns.forEach(([k], j) => {
    const isC = G.carriers.includes(+k)
    const P = pos[k]
    const st: CSSProperties | undefined = selected ? { animation: `opPulse 1.8s ease-in-out ${j * 0.22}s infinite` } : undefined
    kids.push(<rect key={'b' + k} x={P.x} y={P.y} width={bw} height={bh} rx={2} fill={isC ? stroke : 'none'} stroke={isC ? 'none' : stroke} strokeWidth={1.3} style={st} />)
    kids.push(<text key={'t' + k} x={P.x + bw / 2} y={P.y + bh / 2 + 3} textAnchor="middle" fill={isC ? A.carInk : (selected ? A.ac : A.numUnsel)} style={{ font: "600 7px 'IBM Plex Mono'" }}>{k}</text>)
  })

  const sc = 34 / Math.max(W, H)
  return (
    <svg width={Math.round(W * sc * 1.6)} height={Math.round(H * sc * 1.6)} viewBox={`0 0 ${W} ${H}`} style={{ display: 'block' }}>
      {kids}
    </svg>
  )
}
